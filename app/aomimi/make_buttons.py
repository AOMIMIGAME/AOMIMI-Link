"""AOMIMI サーバーのボタンの画像を作る。

公式の Pretendo のボタンの枠（背景・枠線・LOADED の札）をもとに、選んでいるときの紫を水色に変え、
アイコンと文字を AOMIMI のものに描き直す。Pretendo のロゴや名前は残さない。

アイコンは icon_source.png（黒い背景のアオミミガメ）。黒を透明にして、ボタンの真ん中に置く。

使い方: python make_buttons.py （app/romfs/gfx に aomimi_*.png、aomimi/icon_button.png を書き出す）
"""
import colorsys
import os

from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
GFX = os.path.join(HERE, '..', 'romfs', 'gfx')
FONT = 'C:/Windows/Fonts/segoeuib.ttf'

ICON_ROWS = (22, 77)  # * 公式のアイコン（26〜56）と名前（66〜73）の行。ここを描き直す
ICON_BOX = (40, 31)  # * アイコンを収める幅と高さ。公式のアイコンは高さ 28〜31。横長なので幅を少し広く取る
ICON_TOP = 26
HUE_SHIFT = -75 / 360  # * 紫（約 270 度）→ 水色（約 195 度）


def recolor(image):
    out = image.copy()
    pixels = out.load()
    for y in range(out.height):
        for x in range(out.width):
            r, g, b, a = pixels[x, y]
            h, s, v = colorsys.rgb_to_hsv(r / 255, g / 255, b / 255)
            # * 紫っぽい色（背景・枠線）だけ回す。LOADED の緑や灰色はそのまま
            if s > 0.15 and 0.6 < h < 0.9:
                h = (h + HUE_SHIFT) % 1
                r, g, b = (round(c * 255) for c in colorsys.hsv_to_rgb(h, s, v))
                pixels[x, y] = (r, g, b, a)
    return out


def make_icon(source_name='icon_source.png', box=ICON_BOX, out_name='icon_button.png'):
    """黒い背景を透明にする。明るさを不透明さにし、色は明るさで割り戻す（縁のなめらかさを残す）"""
    source = Image.open(os.path.join(HERE, source_name)).convert('RGB')
    mask = source.convert('L').point(lambda v: 255 if v > 40 else 0)
    source = source.crop(mask.getbbox())

    icon = Image.new('RGBA', source.size)
    src = source.load()
    dst = icon.load()
    for y in range(source.height):
        for x in range(source.width):
            r, g, b = src[x, y]
            alpha = max(r, g, b)
            if alpha < 24:
                dst[x, y] = (0, 0, 0, 0)
                continue
            dst[x, y] = (min(255, r * 255 // alpha), min(255, g * 255 // alpha), min(255, b * 255 // alpha), alpha)

    scale = min(box[0] / icon.width, box[1] / icon.height)
    size = (max(1, round(icon.width * scale)), max(1, round(icon.height * scale)))
    icon = icon.resize(size, Image.LANCZOS)
    icon.save(os.path.join(HERE, out_name))
    return icon


def make(variant, icon, out_dir=GFX):
    base = Image.open(os.path.join(GFX, f'pretendo_{variant}.png')).convert('RGBA')
    # * 選んでいないボタンは公式も灰色の枠なので、色はそのまま
    image = recolor(base) if variant.endswith('_selected') and 'deselected' not in variant else base.copy()
    background = image.getpixel((50, 108))
    draw = ImageDraw.Draw(image)
    draw.rectangle((4, ICON_ROWS[0], image.width - 5, ICON_ROWS[1]), fill=background)

    top = ICON_TOP + (ICON_BOX[1] - icon.height) // 2
    image.alpha_composite(icon, ((image.width - icon.width) // 2, top))

    font = ImageFont.truetype(FONT, 12)
    text = 'AOMIMI'
    width = draw.textlength(text, font=font)
    draw.text(((image.width - width) / 2, 62), text, font=font, fill=(255, 255, 255, 255))
    image.save(os.path.join(out_dir, f'aomimi_{variant}.png'))


VARIANTS = ('loaded_selected', 'loaded_deselected', 'unloaded_selected', 'unloaded_deselected')

if __name__ == '__main__':
    import sys
    # * python make_buttons.py a → A のロゴ版を aomimi/variants/a/ に作る（アプリには入れない。見比べ用）
    if len(sys.argv) > 1 and sys.argv[1] == 'a':
        out_dir = os.path.join(HERE, 'variants', 'a')
        os.makedirs(out_dir, exist_ok=True)
        icon = make_icon('icon_source_a.png', ICON_BOX, 'icon_button_a.png')
        for variant in VARIANTS:
            make(variant, icon, out_dir)
    else:
        icon = make_icon()
        for variant in VARIANTS:
            make(variant, icon)
    print('ok', icon.size)
