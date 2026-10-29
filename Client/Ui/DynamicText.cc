#include <Client/Ui/DynamicText.hh>

#include <Client/Game.hh>

using namespace Ui;

DynamicText::DynamicText(float size, std::function<std::string(void)> const &genfunc, Style s)
 : Element(0, size, s), generator(std::move(genfunc))
{
}

void DynamicText::refactor() {
    std::string next = generator();
    if (next == text && width > 0) return;
    text = std::move(next);
    Game::renderer.set_text_size(height);
    width = Game::renderer.get_text_size(text.c_str());
}

void DynamicText::on_render(Renderer &ctx) {
    // Regenerate every paint so animated labels (e.g. title「连接中…」) actually tick.
    // refactor() alone can miss frames when a parent skips layout.
    std::string next = generator();
    if (next != text) {
        text = std::move(next);
        Game::renderer.set_text_size(height);
        width = Game::renderer.get_text_size(text.c_str());
    }
    ctx.draw_text(text.c_str(), {
        .fill = style.fill,
        .size = height,
        .stroke_scale = 0.08f
    });
}
