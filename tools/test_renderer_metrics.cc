// Build with em++ and Renderer.cc, Helpers/{Math,UTF8}.cc; run in a browser.
#include <Client/Render/Renderer.hh>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <emscripten.h>

static void check(Renderer &r, char const *text) {
    float cached=r.get_text_size(text);
    Renderer::flush();
    double actual=EM_ASM_DOUBLE({return Module.ctxs[$0].measureText(UTF8ToString($1)).width;},r.id,text);
    if(std::fabs(cached-actual)>.001) {
        std::printf("FAIL cached=%f actual=%f\n",cached,actual);
        std::abort();
    }
}
int main() {
    EM_ASM({
        Module.ctxs=[new OffscreenCanvas(1,1).getContext('2d')];
        Module.availableCtxs=[]; Module.TextDecoder=new TextDecoder();
        Module['measureCalls']=0;
        const measure=OffscreenCanvasRenderingContext2D.prototype.measureText;
        OffscreenCanvasRenderingContext2D.prototype.measureText=function(t){Module['measureCalls']++;return measure.call(this,t);};
    });
    Renderer a,b;
    Renderer::flush();
    for(auto *r:{&a,&b}) {
        r->set_dimensions(400,100); r->reset();
        check(*r,"中文 English");
        r->set_text_size(17); check(*r,"中文 English");
        { RenderContext saved(r); r->set_text_size(24); check(*r,"中文 English"); }
        check(*r,"中文 English");
        r->draw_text("test",{.size=31}); check(*r,"中文 English");
        r->set_dimensions(401,100); check(*r,"中文 English");
    }
    a.set_text_size(17); check(a,"复用 Cached");
    int before=EM_ASM_INT({return Module['measureCalls'];});
    for(int i=0;i<1000;++i)a.get_text_size("复用 Cached");
    Renderer::flush();
    if(before!=EM_ASM_INT({return Module['measureCalls'];}))std::abort();
    EM_ASM({document.fonts.dispatchEvent(new Event('loadingdone'));});
    Renderer::flush();
    a.get_text_size("复用 Cached");
    if(before+1!=EM_ASM_INT({return Module['measureCalls'];}))std::abort();
    EM_ASM({Module['metricsPassed']=true;});
    std::puts("PASS font size, saved state, draw_text, resize, cross-canvas reuse, 1000 cache hits and font invalidation");
}
