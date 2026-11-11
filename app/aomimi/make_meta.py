"""ホーム画面のアイコン（48x48）と、上の画面の絵（256x128）を AOMIMI のものにする。

公式の Nimbus のアイコンと絵（Pretendo のロゴ）を置き換える。亀は icon_source.png（make_buttons.py と同じ）。

使い方: python make_meta.py （app/meta/icon.png と banner.png を書き換え、romfs/gfx/aomimi_top.png を作る）
"""
import os

from PIL import Image, ImageDraw, ImageFont

from make_buttons import make_icon

HERE = os.path.dirname(os.path.abspath(__file__))
META = os.path.join(HERE, '..', 'meta')
BACKGROUND = (26, 27, 34, 255)  # * ボタンと同じ暗い紺
ACCENT = (77, 140, 255, 255)  # * 亀の耳の青
FONT_BOLD = 'C:/Windows/Fonts/segoeuib.ttf'
FONT = 'C:/Windows/Fonts/segoeui.ttf'


def make_home_icon():
    icon = Image.new('RGBA', (48, 48), BACKGROUND)
    turtle = make_icon(box=(42, 34), out_name='icon_home_turtle.png')
    icon.alpha_composite(turtle, ((48 - turtle.width) // 2, (48 - turtle.height) // 2 + 1))
    icon.save(os.path.join(META, 'icon.png'))


def make_banner():
    banner = Image.new('RGBA', (256, 128), BACKGROUND)
    draw = ImageDraw.Draw(banner)
    # * 左下から右上へ、耳の青のゆるい弧（公式の絵の弧の代わり）
    for radius in (170, 140):
        draw.arc((-radius, 128 - radius, radius, 128 + radius), 270, 360, fill=(31, 40, 60, 255), width=8)

    turtle = make_icon(box=(96, 64), out_name='icon_banner_turtle.png')
    banner.alpha_composite(turtle, (14, (112 - turtle.height) // 2))

    title = ImageFont.truetype(FONT_BOLD, 30)
    sub = ImageFont.truetype(FONT, 9)
    draw.text((116, 22), 'AOMIMI', font=title, fill=(255, 255, 255, 255))
    draw.text((118, 58), 'Link', font=ImageFont.truetype(FONT_BOLD, 18), fill=ACCENT)
    line = 'AOMIMI Network / Pretendo / Nintendo'
    draw.text(((256 - draw.textlength(line, font=sub)) / 2, 108), line, font=sub, fill=(200, 205, 215, 255))
    banner.save(os.path.join(META, 'banner.png'))


if __name__ == '__main__':
    make_home_icon()
    make_banner()
    print('ok')


def make_top():
    """上の画面の真ん中の絵（公式は Pretendo のロゴ、245x76）"""
    top = Image.new('RGBA', (260, 76), (0, 0, 0, 0))
    draw = ImageDraw.Draw(top)
    turtle = make_icon(box=(90, 60), out_name='icon_top_turtle.png')
    top.alpha_composite(turtle, (0, (76 - turtle.height) // 2))
    draw.text((98, 8), 'AOMIMI', font=ImageFont.truetype(FONT_BOLD, 30), fill=(255, 255, 255, 255))
    draw.text((100, 46), 'Link', font=ImageFont.truetype(FONT_BOLD, 18), fill=ACCENT)
    top.save(os.path.join(HERE, '..', 'romfs', 'gfx', 'aomimi_top.png'))


if __name__ == '__main__':
    make_top()
