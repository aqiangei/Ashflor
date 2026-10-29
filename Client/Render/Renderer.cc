#include <Client/Render/Renderer.hh>
#include <Client/Render/CanvasBatch.hh>

#include <Helpers/Macros.hh>
#include <Helpers/Math.hh>
#include <Helpers/UTF8.hh>

#include <cmath>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <emscripten.h>

std::vector<Renderer *> Renderer::renderers;

static void mark_transform_dirty(Renderer *r) {
    r->transform_dirty = true;
}

static void flush_transform(Renderer *r) {
    if (!r->transform_dirty) return;
    r->transform_dirty = false;
    auto const *m = r->context.transform_matrix;
    CanvasBatch::emit(CanvasBatch::Transform, r->id, {m[0],m[1],m[3],m[4],m[2],m[5]});
}

void Renderer::flush() { CanvasBatch::flush(); }

// The renderer owns one baseline context as a member. It is state storage,
// not a saved canvas state, so its destructor must not call restore().
RenderContext::RenderContext() : renderer(nullptr) {}

RenderContext::RenderContext(Renderer *r) {
    // Push a matching canvas/C++ pair — flush any pending setTransform first
    // so save()/restore() cannot resurrect a stale matrix under lazy updates.
    flush_transform(r);
    *this = r->context;
    renderer = r;
    CanvasBatch::emit(CanvasBatch::Save, r->id);
}

void RenderContext::reset() {
    amount = 0;
    color_filter = 0;
    clip_x = renderer->width / 2;
    clip_y = renderer->height / 2;
    //prevents premature unrendering
    clip_w = std::fmax(renderer->width, 10000.0);
    clip_h = std::fmax(renderer->height, 10000.0);
}

RenderContext::~RenderContext() {
    if (renderer == nullptr) return;
    renderer->context = *this;
    CanvasBatch::emit(CanvasBatch::Restore, renderer->id);
    // Canvas and C++ matrix both restored to the saved pair.
    renderer->transform_dirty = false;
}

Renderer::Renderer() : context() {
    CanvasBatch::flush();
    id = EM_ASM_INT({
        let idx;
        if (Module.availableCtxs.length > 0)
            idx = Module.availableCtxs.pop();
        else
            idx = Module.ctxs.length;
#ifdef ART_STANDALONE
        // Codex / art module: never bind #canvas (page may not have one).
        {
            const canvas = new OffscreenCanvas(1,1);
            Module.ctxs[idx] = canvas.getContext('2d', { alpha: true });
        }
#else
        if (idx === 0) {
            // Opaque: cleared frames composite as solid (still fill void after resize).
            Module.ctxs[idx] = document.getElementById('canvas').getContext('2d', { alpha: false });
        } else {
            // Must request alpha — bare getContext('2d') can yield an opaque
            // buffer; Mob/Petal raster caches then blit as solid squares
            // (training dummy looked like a boxed target tile).
            const canvas = new OffscreenCanvas(1,1);
            Module.ctxs[idx] = canvas.getContext('2d', { alpha: true });
        }
#endif
        return idx;
    });
    DEBUG_ONLY(std::cout << "created canvas " << id << '\n';)
    Renderer::renderers.push_back(this);
    context.renderer = this;
    context.reset();
}

Renderer::~Renderer() {
    // The member context is baseline state, not an active save/restore guard.
    // Its destructor runs after this body, once the canvas slot is released.
    context.renderer = nullptr;
    auto it = std::find(renderers.begin(), renderers.end(), this);
    if (it != renderers.end()) renderers.erase(it);
    CanvasBatch::flush();
    EM_ASM({
        if ($0 == 0)
            throw new Error('Tried to delete the main context');
        Module.ctxs[$0] = null;
        Module.availableCtxs.push($0);
    }, id);
    DEBUG_ONLY(std::cout << "removed canvas " << id << '\n';)
}

uint32_t Renderer::HSV(uint32_t c, float v) {
    return MIX(c >> 24 << 24, c, v);
}

uint32_t Renderer::MIX(uint32_t base, uint32_t mix, float v) {
    uint8_t b = fclamp((mix & 255) * v + (base & 255) * (1 - v), 0, 255);
    uint8_t g = fclamp(((mix >> 8) & 255) * v + ((base >> 8) & 255) * (1 - v), 0, 255);
    uint8_t r = fclamp(((mix >> 16) & 255) * v + ((base >> 16) & 255) * (1 - v), 0, 255);
    uint8_t a = base >> 24;
    return (a << 24) | (r << 16) | (g << 8) | b;
}

void Renderer::reset() {
    reset_transform();
    set_global_alpha(1.f);
    round_line_cap();
    round_line_join();
    center_text_align();
    center_text_baseline();
    context.reset();
}

void Renderer::clear() {
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::Clear, id, {width,height});
}

void Renderer::set_dimensions(float w, float h) {
    if (width == w && height == h) return;
    width = w;
    height = h;
    CanvasBatch::flush();
    EM_ASM({
        Module.ctxs[$0].canvas.width = $1;
        Module.ctxs[$0].canvas.height = $2;
    }, id, w, h);
}

void Renderer::add_color_filter(uint32_t c, float v) {
    context.color_filter = c;
    context.amount = v;
}

void Renderer::set_fill(uint32_t v) {
    v = MIX(v, context.color_filter, context.amount);
    CanvasBatch::emit(CanvasBatch::FillStyle, id, {(double)v});
}

void Renderer::set_stroke(uint32_t v) {
    v = MIX(v, context.color_filter, context.amount);
    CanvasBatch::emit(CanvasBatch::StrokeStyle, id, {(double)v});
}

void Renderer::set_line_width(float v) {
    CanvasBatch::emit(CanvasBatch::LineWidth, id, {v});
}

void Renderer::set_text_size(float v) {
    CanvasBatch::emit(CanvasBatch::Font, id, {v});
}

void Renderer::set_global_alpha(float v) {
    CanvasBatch::emit(CanvasBatch::Alpha, id, {v});
}

void Renderer::round_line_cap() {
    CanvasBatch::emit(CanvasBatch::Cap, id);
}

void Renderer::round_line_join() {
    CanvasBatch::emit(CanvasBatch::Join, id);
}

void Renderer::center_text_align() {
    CanvasBatch::emit(CanvasBatch::Align, id);
}

void Renderer::center_text_baseline() {
    CanvasBatch::emit(CanvasBatch::Baseline, id);
}

void Renderer::set_transform(float a, float b, float c, float d, float e, float f) {
    context.transform_matrix[0] = a;
    context.transform_matrix[1] = b;
    context.transform_matrix[2] = c;
    context.transform_matrix[3] = d;
    context.transform_matrix[4] = e;
    context.transform_matrix[5] = f;
    mark_transform_dirty(this);
}

void Renderer::scale(float v) {
    context.transform_matrix[0] *= v;
    context.transform_matrix[1] *= v;
    context.transform_matrix[3] *= v;
    context.transform_matrix[4] *= v;
    mark_transform_dirty(this);
}

void Renderer::scale(float x, float y) {
    context.transform_matrix[0] *= x;
    context.transform_matrix[1] *= x;
    context.transform_matrix[3] *= y;
    context.transform_matrix[4] *= y;
    mark_transform_dirty(this);
}

void Renderer::translate(float x, float y) {
    context.transform_matrix[2] += x * context.transform_matrix[0] + y * context.transform_matrix[3];
    context.transform_matrix[5] += y * context.transform_matrix[4] + x * context.transform_matrix[1];
    mark_transform_dirty(this);
}

void Renderer::rotate(float a) {
    float cos_a = cosf(a);
    float sin_a = sinf(a);
    float original0 = context.transform_matrix[0];
    float original1 = context.transform_matrix[1];
    float original3 = context.transform_matrix[3];
    float original4 = context.transform_matrix[4];
    context.transform_matrix[0] = original0 * cos_a + original1 * -sin_a;
    context.transform_matrix[1] = original0 * sin_a + original1 * cos_a;
    context.transform_matrix[3] = original3 * cos_a + original4 * -sin_a;
    context.transform_matrix[4] = original3 * sin_a + original4 * cos_a;
    mark_transform_dirty(this);
}

void Renderer::reset_transform() {
    set_transform(1,0,0,0,1,0);
}

void Renderer::begin_path() {
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::Begin, id);
}

void Renderer::move_to(float x, float y) {
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::Move, id, {x,y});
}

void Renderer::line_to(float x, float y) {
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::Line, id, {x,y});
}

void Renderer::qcurve_to(float x, float y, float x1, float y1) {
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::Quadratic, id, {x,y,x1,y1});
}

void Renderer::bcurve_to(float x, float y, float x1, float y1, float x2, float y2) {
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::Bezier, id, {x,y,x1,y1,x2,y2});
}


void Renderer::partial_arc(float x, float y, float r, float sa, float ea, uint8_t ccw) {
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::Arc, id, {x,y,r,sa,ea,(double)ccw});
}

void Renderer::arc(float x, float y, float r) {
    partial_arc(x, y, r, 0, 2 * M_PI, 0);
}

void Renderer::reverse_arc(float x, float y, float r) {
    partial_arc(x, y, r, 0, 2 * M_PI, 1);
}

void Renderer::ellipse(float x, float y, float r1, float r2, float a) {
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::Ellipse, id, {x,y,r1,r2,a});
}

void Renderer::ellipse(float x, float y, float r1, float r2) {
    ellipse(x, y, r1, r2, 0);
}

void Renderer::fill_rect(float x, float y, float w, float h) {
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::FillRect, id, {x,y,w,h});
}

void Renderer::stroke_rect(float x, float y, float w, float h) {
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::StrokeRect, id, {x,y,w,h});
}

void Renderer::rect(float x, float y, float w, float h) {
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::Rect, id, {x,y,w,h});
}

void Renderer::round_rect(float x, float y, float w, float h, float r) {
    move_to(x + r, y);
    line_to(x + w - r, y);
    qcurve_to(x + w, y, x + w, y + r);
    line_to(x + w, y + h - r);
    qcurve_to(x + w, y + h, x + w - r, y + h);
    line_to(x + r, y + h);
    qcurve_to(x, y + h, x, y + h - r);
    line_to(x, y + r);
    qcurve_to(x, y, x + r, y);
}

void Renderer::close_path() {
    CanvasBatch::emit(CanvasBatch::Close, id);
}

void Renderer::fill(uint8_t o) {
    CanvasBatch::emit(CanvasBatch::Fill, id, {(double)o});
}

void Renderer::stroke() {
    CanvasBatch::emit(CanvasBatch::Stroke, id);
}

void Renderer::clip() {
    CanvasBatch::emit(CanvasBatch::Clip, id);
}

void Renderer::clip_rect(float x, float y, float w, float h) {
    flush_transform(this);
    //assumes axis oriented scaling
    context.clip_x = x * context.transform_matrix[0] + context.transform_matrix[2];
    context.clip_w = w * context.transform_matrix[0];
    context.clip_y = y * context.transform_matrix[4] + context.transform_matrix[5];
    context.clip_h = h * context.transform_matrix[4];
    begin_path();
    rect(x-w/2,y-h/2,w,h);
    clip();
}

void Renderer::draw_image(Renderer &ctx) {
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::ImageCanvas,id,{(double)ctx.id,-ctx.width/2,-ctx.height/2});
}

void Renderer::draw_image_data(char const *data, float width, float height) {
    (void)try_draw_image_data(data, width, height);
}

uint8_t Renderer::try_draw_image_data(char const *data, float width, float height) {
    static std::unordered_map<std::string,int> handles;
    auto it=handles.find(data);
    int handle;
    if(it==handles.end()) {
        handle=EM_ASM_INT({
            const key=UTF8ToString($0);
            if(!Module.embeddedImages)Module.embeddedImages=new Map();
            let img=Module.embeddedImages.get(key);
            if(!img){img=new Image();img.src=key;Module.embeddedImages.set(key,img);}
            if(!img.complete||!img.naturalWidth)return -1;
            if(!Module['imageHandles'])Module['imageHandles']=[];
            Module['imageHandles'].push(img);
            return Module['imageHandles'].length-1;
        },data);
        if(handle<0)return 0;
        handles.emplace(data,handle);
    } else handle=it->second;
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::ImageBitmap,id,{(double)handle,width,height});
    return 1;
}

void Renderer::draw_image_at(Renderer &ctx, float x, float y) {
    flush_transform(this);
    CanvasBatch::emit(CanvasBatch::ImageCanvas,id,{(double)ctx.id,x,y});
}

void Renderer::fill_text(char const *text) {
    flush_transform(this);
    auto const &owned=CanvasBatch::texts().emplace_back(text);
    CanvasBatch::emit(CanvasBatch::FillText,id,{(double)(uintptr_t)owned.data(),(double)owned.size()});
}

void Renderer::stroke_text(char const *text) {
    flush_transform(this);
    auto const &owned=CanvasBatch::texts().emplace_back(text);
    CanvasBatch::emit(CanvasBatch::StrokeText,id,{(double)(uintptr_t)owned.data(),(double)owned.size()});
}

void Renderer::draw_text(char const *text, struct TextArgs const args) {
    flush_transform(this);
    auto const &owned=CanvasBatch::texts().emplace_back(text);
    uint32_t const fill=MIX(args.fill,context.color_filter,context.amount);
    uint32_t const stroke=MIX(args.stroke,context.color_filter,context.amount);
    CanvasBatch::emit(CanvasBatch::Text,id,{(double)(uintptr_t)owned.data(),(double)owned.size(),
        (double)fill,(double)stroke,args.size,args.stroke_scale>0?args.size*args.stroke_scale:0.0});
}

float Renderer::get_text_size(char const *text) {
    flush_transform(this);
    CanvasBatch::flush();
    return EM_ASM_DOUBLE({
        return Module.ctxs[$0].measureText(Module.TextDecoder.decode(HEAPU8.subarray($1, $1+$2)),0,0).width;
    }, id, text, std::strlen(text));
}

//precalculated ascii for standard Ubuntu font, only serves as an approximation (will fail for certain scripts)
constexpr float CHAR_WIDTHS[128] = {0,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0,0.24,0.24,0.24,0.24,0.24,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0,0.5,0.5,0.24,0.286,0.465,0.699,0.568,0.918,0.705,0.247,0.356,0.356,0.502,0.568,0.246,0.34,0.246,0.437,0.568,0.568,0.568,0.568,0.568,0.568,0.568,0.568,0.568,0.568,0.246,0.246,0.568,0.568,0.568,0.455,0.974,0.721,0.672,0.648,0.737,0.606,0.574,0.702,0.734,0.316,0.529,0.684,0.563,0.897,0.756,0.79,0.644,0.79,0.667,0.582,0.614,0.707,0.722,0.948,0.675,0.661,0.61,0.371,0.437,0.371,0.568,0.5,0.286,0.553,0.604,0.5,0.604,0.584,0.422,0.594,0.589,0.289,0.289,0.579,0.316,0.862,0.589,0.607,0.604,0.604,0.422,0.485,0.444,0.589,0.55,0.784,0.554,0.547,0.5,0.371,0.322,0.371,0.568,0.5};

float Renderer::get_ascii_text_size(char const *text) {
    float w = 0;
    UTF8Parser parser(text, std::strlen(text));
    while (1) {
        uint32_t c = parser.next_symbol();
        if (c == 0) break;
        if (c < 128)
            w += CHAR_WIDTHS[c];
        else
            w += 1;
    }
    return w;
}
