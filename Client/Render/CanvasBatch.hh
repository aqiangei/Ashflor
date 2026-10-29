#pragma once
#include <vector>
#include <initializer_list>
#include <deque>
#include <string>
#include <emscripten.h>

// Preserve Canvas order and geometry, but cross the WASM/JS boundary once for
// a sequence of path/state operations. Text, images and readbacks are barriers.
namespace CanvasBatch {
enum Op { Save, Restore, Transform, Begin, Move, Line, Quadratic, Bezier, Arc,
          Ellipse, FillRect, StrokeRect, Rect, Close, Fill, Stroke, Clip,
          FillStyle, StrokeStyle, LineWidth, Font, Alpha, Cap, Join, Align, Baseline, Clear,
          ImageCanvas, ImageBitmap, FillText, StrokeText, Text };

EM_JS(void, execute, (double const *ptr, int count), {
    var a = HEAPF64, i = ptr / 8, end = i + count;
    function color(v) {
        v = v >>> 0;
        return 'rgba(' + ((v >>> 16) & 255) + ',' + ((v >>> 8) & 255)
            + ',' + (v & 255) + ',' + ((v >>> 24) / 255) + ')';
    }
    while (i < end) {
        var c = Module.ctxs[a[i++]], op = a[i++];
        switch (op) {
        case 0: c.save(); break;
        case 1: c.restore(); break;
        case 2: c.setTransform(a[i],a[i+1],a[i+2],a[i+3],a[i+4],a[i+5]); i+=6; break;
        case 3: c.beginPath(); break;
        case 4: c.moveTo(a[i],a[i+1]); i+=2; break;
        case 5: c.lineTo(a[i],a[i+1]); i+=2; break;
        case 6: c.quadraticCurveTo(a[i],a[i+1],a[i+2],a[i+3]); i+=4; break;
        case 7: c.bezierCurveTo(a[i],a[i+1],a[i+2],a[i+3],a[i+4],a[i+5]); i+=6; break;
        case 8: c.arc(a[i],a[i+1],a[i+2],a[i+3],a[i+4],!!a[i+5]); i+=6; break;
        case 9: c.ellipse(a[i],a[i+1],a[i+2],a[i+3],a[i+4],2*Math.PI,0); i+=5; break;
        case 10: c.fillRect(a[i],a[i+1],a[i+2],a[i+3]); i+=4; break;
        case 11: c.strokeRect(a[i],a[i+1],a[i+2],a[i+3]); i+=4; break;
        case 12: c.rect(a[i],a[i+1],a[i+2],a[i+3]); i+=4; break;
        case 13: c.closePath(); break;
        case 14: c.fill(a[i++] ? 'nonzero' : 'evenodd'); break;
        case 15: c.stroke(); break;
        case 16: c.clip(); break;
        case 17: c.fillStyle = color(a[i++]); break;
        case 18: c.strokeStyle = color(a[i++]); break;
        case 19: c.lineWidth = a[i++]; break;
        case 20: c.font = '800 ' + a[i++] + 'px "Noto Sans SC", "Microsoft YaHei", "Segoe UI", sans-serif'; break;
        case 21: c.globalAlpha = a[i++]; break;
        case 22: c.lineCap = 'round'; break;
        case 23: c.lineJoin = 'round'; break;
        case 24: c.textAlign = 'center'; break;
        case 25: c.textBaseline = 'middle'; break;
        case 26: c.clearRect(0,0,a[i],a[i+1]); i+=2; break;
        case 27: c.drawImage(Module.ctxs[a[i]].canvas,a[i+1],a[i+2]); i+=3; break;
        case 28: {
            var img = Module['imageHandles'][a[i]], w=a[i+1], h=a[i+2]; i+=3;
            var s = Math.min(w/img.naturalWidth,h/img.naturalHeight);
            w=img.naturalWidth*s; h=img.naturalHeight*s;
            c.drawImage(img,-w/2,-h/2,w,h); break;
        }
        case 29: case 30: {
            var text=Module.TextDecoder.decode(HEAPU8.subarray(a[i],a[i]+a[i+1])); i+=2;
            if(op===29)c.fillText(text,0,0);else c.strokeText(text,0,0);break;
        }
        case 31: {
            var text=Module.TextDecoder.decode(HEAPU8.subarray(a[i],a[i]+a[i+1]));
            c.fillStyle=color(a[i+2]); c.strokeStyle=color(a[i+3]);
            c.font='800 '+a[i+4]+'px "Noto Sans SC", "Microsoft YaHei", "Segoe UI", sans-serif';
            if(a[i+5]>0){c.lineWidth=a[i+5];c.strokeText(text,0,0);}
            c.fillText(text,0,0); i+=6;break;
        }
        default: throw new Error('Invalid Canvas command: ' + op);
        }
    }
    var stats = Module['canvasBatchStats'] || (Module['canvasBatchStats'] = {'calls':0, 'values':0});
    stats['calls']++; stats['values'] += count;
});

// Renderers can be global objects: function statics avoid cross-TU startup order.
inline std::vector<double> &queue() { static std::vector<double> v; return v; }
inline std::deque<std::string> &texts() { static std::deque<std::string> v; return v; }
inline void flush() {
    auto &commands = queue();
    if (commands.empty()) return;
    execute(commands.data(), (int)commands.size());
    commands.clear();
    texts().clear();
}
inline void emit(Op op, unsigned id, std::initializer_list<double> args = {}) {
    auto &commands = queue();
    commands.push_back(id);
    commands.push_back(op);
    commands.insert(commands.end(), args);
    // Bound temporary memory even for a large procedural monster / map bake.
    if (commands.size() >= 32768) flush();
}
}
