import os
import shutil
from PIL import Image, ImageDraw, ImageFont

SRC1 = r"S:\Sync\Programmation\C\TPE\build-TPE_NEW-Desktop_Qt_5_3_MinGW_32bit-Debug\images"
SRC2 = r"S:\Sync\Programmation\C\TPE\build-QT_and_SDL-Desktop_Qt_5_3_MinGW_32bit-Debug\images"
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DEST_IMAGES = os.path.join(ROOT, "images")
DEST_ASSETS = os.path.join(ROOT, "assets")

os.makedirs(DEST_IMAGES, exist_ok=True)
os.makedirs(DEST_ASSETS, exist_ok=True)

def copy_to_both(src_path, filename):
    d1 = os.path.join(DEST_IMAGES, filename)
    d2 = os.path.join(DEST_ASSETS, filename)
    shutil.copy2(src_path, d1)
    shutil.copy2(src_path, d2)
    print(f"Copied {filename} to images/ and assets/")

def save_image_to_both(im, filename, **kwargs):
    d1 = os.path.join(DEST_IMAGES, filename)
    d2 = os.path.join(DEST_ASSETS, filename)
    im.save(d1, **kwargs)
    im.save(d2, **kwargs)
    print(f"Saved {filename} ({im.size}, {im.mode}) to images/ and assets/")

# 1. Direct copy of authentic original images
direct_copies_src1 = [
    "ENR5.png",
    "Nucléaire.png",
    "stats.png",
    "potentiel.png",
    "eoliennes_taille.png",
    "Eo_puiss.gif",
    "fileopen.png",
    "icone.png",
    "icone_quit.png",
    "icone_save.png",
    "nuclo.png",
    "nuclo1.jpg",
    "Photo.png",
    "pico.png",
]

for name in direct_copies_src1:
    p = os.path.join(SRC1, name)
    if os.path.exists(p):
        copy_to_both(p, name)
    else:
        print(f"WARNING: {p} not found!")

# Export Eo_puiss.png from Eo_puiss.gif for Qt or viewers
eo_gif = Image.open(os.path.join(SRC1, "Eo_puiss.gif"))
save_image_to_both(eo_gif.convert("RGB"), "Eo_puiss.png", format="PNG")

# Direct copies from SRC2 (SDL / presentation)
direct_copies_src2 = [
    "robot.bmp",
    "robot.jpg",
    "retour.bmp",
    "eolienne.gif",
    "eolienne_fixe.png",
    "bulle1.bmp",
    "bulle1.png",
    "bulle2.png",
    "bulle3.png",
    "bulle4.png",
    "bulle5.png",
    "bulle6.png",
    "bulle_vierge.png",
]

for name in direct_copies_src2:
    p = os.path.join(SRC2, name)
    if os.path.exists(p):
        copy_to_both(p, name)
    else:
        print(f"WARNING: {p} not found!")

# Convert all bubbles to 24-bit BMP with white background for SDL_LoadBMP
for i in range(1, 7):
    png_name = f"bulle{i}.png"
    bmp_name = f"bulle{i}.bmp"
    src_p = os.path.join(SRC2, png_name)
    if os.path.exists(src_p):
        b_im = Image.open(src_p).convert("RGBA")
        # composite over pure white background
        bg = Image.new("RGB", b_im.size, (255, 255, 255))
        bg.paste(b_im, (0, 0), b_im)
        save_image_to_both(bg, bmp_name, format="BMP")

# 2. Process and uniformize FOND.png and FOND.bmp
print("\nProcessing and uniformizing FOND.png...")
fond_src = Image.open(os.path.join(SRC2, "FOND.png")).convert("RGBA")
# Crop away the top 11px and bottom 16px white strips (805 px height)
cropped_fond = fond_src.crop((0, 11, 1040, 816))
# Resize back cleanly to (1040, 832) using high-quality LANCZOS
uniform_fond = cropped_fond.resize((1040, 832), Image.Resampling.LANCZOS)
save_image_to_both(uniform_fond, "FOND.png", format="PNG")

# Also save 24-bit BMP version for SDL
fond_bmp = Image.new("RGB", (1040, 832), (255, 255, 255))
fond_bmp.paste(uniform_fond, (0, 0), uniform_fond)
save_image_to_both(fond_bmp, "FOND.bmp", format="BMP")

# 3. Process credits3.png (for Qt dialog) with "Créé par fromtherags en 2014"
print("\nProcessing credits3.png...")
cr3 = Image.open(os.path.join(SRC1, "credits3.png")).convert("RGBA")
w3, h3 = cr3.size
# Reconstruct background between y=384 and y=438 by vertical interpolation
y1, y2 = 384, 438
for x in range(30, 850):
    c1 = cr3.getpixel((x, y1))
    c2 = cr3.getpixel((x, y2))
    for y in range(y1, y2):
        t = (y - y1) / float(y2 - y1)
        r = int(c1[0] * (1 - t) + c2[0] * t)
        g = int(c1[1] * (1 - t) + c2[1] * t)
        b = int(c1[2] * (1 - t) + c2[2] * t)
        a = int(c1[3] * (1 - t) + c2[3] * t)
        cr3.putpixel((x, y), (r, g, b, a))

draw3 = ImageDraw.Draw(cr3)
font_path = os.path.join(ROOT, "assets", "PatrickHand-Regular.ttf")
if not os.path.exists(font_path):
    font_path = os.path.join(ROOT, "assets", "DroidSans.ttf")

try:
    font3 = ImageFont.truetype(font_path, 28)
except Exception:
    font3 = ImageFont.load_default()

text = "Créé par fromtherags en 2014"
cx = w3 // 2
cy = (y1 + y2) // 2
# Draw subtle shadow then text
for ox, oy in [(-1, -1), (1, -1), (-1, 1), (1, 1), (0, 2)]:
    draw3.text((cx + ox, cy + oy), text, fill=(0, 0, 0, 220), font=font3, anchor="mm")
draw3.text((cx, cy), text, fill=(255, 255, 255, 255), font=font3, anchor="mm")
save_image_to_both(cr3, "credits3.png", format="PNG")

# 4. Process credits.png (1600x832 for SDL presentation)
print("\nProcessing credits.png...")
cr = Image.open(os.path.join(SRC2, "credits.png")).convert("RGBA")
w, h = cr.size
# In credits.png, names row is between y=542 and y=574
cy1, cy2 = 542, 574
for x in range(100, 1500):
    c1 = cr.getpixel((x, cy1))
    c2 = cr.getpixel((x, cy2))
    for y in range(cy1, cy2):
        t = (y - cy1) / float(cy2 - cy1)
        r = int(c1[0] * (1 - t) + c2[0] * t)
        g = int(c1[1] * (1 - t) + c2[1] * t)
        b = int(c1[2] * (1 - t) + c2[2] * t)
        a = int(c1[3] * (1 - t) + c2[3] * t)
        cr.putpixel((x, y), (r, g, b, a))

draw = ImageDraw.Draw(cr)
try:
    font_cr = ImageFont.truetype(font_path, 34)
except Exception:
    font_cr = ImageFont.load_default()

cx = w // 2
cy = (cy1 + cy2) // 2
for ox, oy in [(-1, -1), (1, -1), (-1, 1), (1, 1), (0, 2)]:
    draw.text((cx + ox, cy + oy), text, fill=(0, 0, 0, 220), font=font_cr, anchor="mm")
draw.text((cx, cy), text, fill=(255, 255, 255, 255), font=font_cr, anchor="mm")
save_image_to_both(cr, "credits.png", format="PNG")

# Also save credits.bmp (24-bit RGB) for SDL_LoadBMP
cr_bmp = Image.new("RGB", cr.size, (0, 0, 0))
cr_bmp.paste(cr, (0, 0), cr)
save_image_to_both(cr_bmp, "credits.bmp", format="BMP")

print("\nAll assets successfully restored and uniformized!")
