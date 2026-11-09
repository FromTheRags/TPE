import os
import math
from PIL import Image, ImageDraw, ImageFont

ASSET_DIR = "images"
FONT_PATH = "assets/DroidSans.ttf"
FONT_BOLD_PATH = "assets/DroidSans-Bold.ttf"

os.makedirs(ASSET_DIR, exist_ok=True)
os.makedirs("assets", exist_ok=True)

def get_font(size, bold=False):
    path = FONT_BOLD_PATH if bold else FONT_PATH
    try:
        return ImageFont.truetype(path, size)
    except:
        return ImageFont.load_default()

print("Generating royalty-free assets...")

# =====================================================================
# 1. ENR5.png (830 x 528) - Central Energy Diagram
# =====================================================================
def create_enr5():
    w, h = 830, 528
    im = Image.new("RGBA", (w, h), (255, 255, 255, 255))
    draw = ImageDraw.Draw(im)

    # 1. Sky Gradient (y: 0 -> 320)
    for y in range(320):
        t = y / 320.0
        r = int(140 + (220 - 140) * t)
        g = int(190 + (240 - 190) * t)
        b = int(240 + (255 - 240) * t)
        draw.line([(0, y), (w, y)], fill=(r, g, b, 255))

    # Sun
    sun_x, sun_y = 520, 60
    for rad in range(45, 0, -1):
        alpha = int(255 * (1.0 - rad/55.0))
        draw.ellipse([sun_x - rad, sun_y - rad, sun_x + rad, sun_y + rad], fill=(255, 220, 80, alpha))
    draw.ellipse([sun_x - 22, sun_y - 22, sun_x + 22, sun_y + 22], fill=(255, 245, 160, 255))

    # Clouds
    def draw_cloud(cx, cy, s):
        for dx, dy, r in [(-25*s, 0, 20*s), (0, -10*s, 26*s), (25*s, 0, 18*s), (-10*s, 8*s, 16*s), (15*s, 6*s, 16*s)]:
            draw.ellipse([cx+dx-r, cy+dy-r, cx+dx+r, cy+dy+r], fill=(255, 255, 255, 220))
    draw_cloud(120, 50, 1.0)
    draw_cloud(380, 45, 0.8)
    draw_cloud(720, 65, 0.9)

    # Mountain range on the right/center horizon
    mountains = [
        [(450, 260), (510, 150), (580, 260)],
        [(550, 260), (630, 110), (710, 260)],
        [(680, 260), (750, 130), (830, 260)]
    ]
    for pts in mountains:
        draw.polygon(pts, fill=(130, 150, 175, 255))
        # snow caps
        peak = pts[1]
        snow_pts = [peak, ((peak[0]+pts[0][0])//2, (peak[1]+pts[0][1])//2 + 10), ((peak[0]+pts[2][0])//2, (peak[1]+pts[2][1])//2 + 10)]
        draw.polygon(snow_pts, fill=(245, 250, 255, 255))

    # Rolling green hills (middle & foreground)
    draw.polygon([(0, 280), (250, 230), (500, 270), (830, 240), (830, 528), (0, 528)], fill=(110, 180, 85, 255))
    draw.polygon([(0, 340), (320, 290), (620, 330), (830, 300), (830, 528), (0, 528)], fill=(95, 165, 75, 255))
    draw.polygon([(0, 420), (380, 390), (830, 370), (830, 528), (0, 528)], fill=(80, 145, 60, 255))

    # Underground / Soil layer for Geothermal (bottom left: 0..380, 440..528)
    draw.polygon([(0, 450), (370, 435), (370, 528), (0, 528)], fill=(120, 80, 50, 255))
    draw.polygon([(0, 475), (370, 465), (370, 528), (0, 528)], fill=(85, 50, 30, 255))

    # River / Reservoir for Hydro (from mountain dam down to center right)
    draw.polygon([(650, 180), (780, 175), (830, 220), (644, 225)], fill=(70, 140, 210, 255))

    # Helper for drawing rounded badge labels
    def draw_badge(x, y, text, color, bg=(255, 255, 255, 235), border=(50, 50, 50, 200)):
        fnt = get_font(13, bold=True)
        bbox = fnt.getbbox(text)
        tw = bbox[2] - bbox[0]
        th = bbox[3] - bbox[1]
        pad_x, pad_y = 8, 4
        rect = [x, y, x + tw + pad_x*2, y + th + pad_y*2]
        draw.rounded_rectangle(rect, radius=6, fill=bg, outline=border, width=2)
        draw.text((x + pad_x, y + pad_y - bbox[1]), text, font=fnt, fill=color)

    # -------------------------------------------------------------
    # Region 0: Éoliennes (201, 120, 216, 115)
    # -------------------------------------------------------------
    # Wind turbines
    def draw_turbine(tx, ty, scale=1.0):
        # mast
        draw.polygon([(tx-2*scale, ty), (tx+2*scale, ty), (tx+4*scale, ty+70*scale), (tx-4*scale, ty+70*scale)], fill=(230, 235, 240, 255))
        # hub
        draw.ellipse([tx-4*scale, ty-4*scale, tx+4*scale, ty+4*scale], fill=(200, 205, 210, 255))
        # blades
        for angle in [0, 120, 240]:
            rad = math.radians(angle + 20)
            bx = tx + math.sin(rad) * 38 * scale
            by = ty - math.cos(rad) * 38 * scale
            draw.line([(tx, ty), (bx, by)], fill=(245, 245, 250, 255), width=int(3*scale))
    draw_turbine(250, 140, 0.75)
    draw_turbine(320, 130, 0.95)
    draw_turbine(380, 145, 0.8)
    draw_badge(210, 125, "1. Éoliennes", (15, 120, 30))

    # -------------------------------------------------------------
    # Region 1: Centrale nucléaire (352, 264, 206, 125)
    # -------------------------------------------------------------
    # Cooling tower 1
    def draw_cooling_tower(cx, cy, cw, ch):
        # Curved cooling tower silhouette
        pts = [(cx, cy+ch), (cx+cw, cy+ch), (cx+int(cw*0.8), cy+int(ch*0.3)), (cx+int(cw*0.85), cy), (cx+int(cw*0.15), cy), (cx+int(cw*0.2), cy+int(ch*0.3))]
        draw.polygon(pts, fill=(210, 215, 220, 255), outline=(130, 135, 140, 255))
        # Steam plume
        for sy in range(4):
            draw.ellipse([cx+10-sy*4, cy-20-sy*10, cx+cw-10+sy*6, cy-5-sy*6], fill=(255, 255, 255, 160-sy*30))
    draw_cooling_tower(365, 290, 50, 75)
    draw_cooling_tower(430, 295, 45, 70)
    # Reactor containment dome
    draw.chord([485, 300, 545, 365], 180, 360, fill=(185, 190, 200, 255), outline=(120, 125, 135, 255))
    draw.rectangle([485, 332, 545, 365], fill=(185, 190, 200, 255), outline=(120, 125, 135, 255))
    draw_badge(360, 270, "2. Centrale Nucléaire", (180, 100, 0))

    # -------------------------------------------------------------
    # Region 2: Panneaux photovoltaïques (557, 377, 272, 113)
    # -------------------------------------------------------------
    for row in range(3):
        ry = 390 + row * 26
        for col in range(4):
            rx = 570 + col * 62 + row * 8
            # slanted solar panel
            pts = [(rx, ry+18), (rx+50, ry+18), (rx+58, ry), (rx+8, ry)]
            draw.polygon(pts, fill=(25, 55, 125, 255), outline=(180, 190, 210, 255), width=2)
            # grid lines
            draw.line([(rx+25, ry+18), (rx+33, ry)], fill=(90, 130, 190, 255))
            draw.line([(rx+4, ry+9), (rx+54, ry+9)], fill=(90, 130, 190, 255))
    draw_badge(565, 382, "3. Solaire Photovoltaïque", (10, 70, 160))

    # -------------------------------------------------------------
    # Region 3: Centrale thermique (0, 190, 183, 140)
    # -------------------------------------------------------------
    # Main building
    draw.rectangle([15, 245, 115, 315], fill=(135, 125, 120, 255), outline=(70, 65, 60, 255))
    # Tall chimneys
    draw.rectangle([35, 195, 47, 245], fill=(160, 70, 60, 255), outline=(60, 50, 50, 255))
    draw.rectangle([65, 195, 77, 245], fill=(160, 70, 60, 255), outline=(60, 50, 50, 255))
    # Smoke puffs
    for i in range(3):
        draw.ellipse([30-i*4, 185-i*10, 50+i*6, 200-i*6], fill=(220, 220, 220, 150-i*35))
        draw.ellipse([60-i*4, 185-i*10, 80+i*6, 200-i*6], fill=(220, 220, 220, 150-i*35))
    draw_badge(10, 200, "4. Centrale Thermique", (120, 50, 40))

    # -------------------------------------------------------------
    # Region 4: Centrale géothermique (134, 412, 228, 116)
    # -------------------------------------------------------------
    # Surface facility
    draw.rectangle([160, 420, 240, 460], fill=(170, 175, 185, 255), outline=(80, 85, 95, 255))
    # Piping into deep ground
    draw.line([(180, 460), (180, 520)], fill=(220, 50, 30, 255), width=4)
    draw.line([(220, 460), (220, 520)], fill=(40, 120, 220, 255), width=4)
    # Underground magma glow
    for gr in range(40, 0, -5):
        draw.ellipse([200-gr, 510-gr//2, 200+gr, 510+gr//2], fill=(240, 90, 30, int(180*(1-gr/45))))
    draw_badge(145, 418, "5. Géothermie", (180, 60, 20))

    # -------------------------------------------------------------
    # Region 5: Centrale hydroélectrique (644, 125, 186, 101)
    # -------------------------------------------------------------
    # Concrete dam structure
    draw.polygon([(670, 160), (790, 150), (810, 215), (680, 225)], fill=(160, 165, 175, 255), outline=(100, 105, 115, 255))
    # Water chutes
    draw.line([(710, 175), (715, 220)], fill=(255, 255, 255, 220), width=4)
    draw.line([(740, 172), (745, 218)], fill=(255, 255, 255, 220), width=4)
    draw_badge(650, 130, "6. Hydroélectricité", (0, 110, 170))

    # -------------------------------------------------------------
    # Region 6: Biomasse (644, 226, 186, 108)
    # -------------------------------------------------------------
    # Silo and cogeneration unit
    draw.rectangle([665, 255, 705, 310], fill=(200, 190, 170, 255), outline=(100, 95, 80, 255))
    draw.chord([665, 240, 705, 270], 180, 360, fill=(180, 170, 150, 255), outline=(100, 95, 80, 255))
    draw.rectangle([715, 270, 775, 310], fill=(130, 160, 120, 255), outline=(60, 80, 50, 255))
    # Green leaf icon
    draw.ellipse([735, 280, 755, 298], fill=(50, 160, 60, 255))
    draw_badge(655, 232, "7. Biomasse", (40, 130, 40))

    im.save(os.path.join(ASSET_DIR, "ENR5.png"))
    print("-> Generated ENR5.png")

create_enr5()

# =====================================================================
# 2. Nucléaire.png (900 x 900) - Map of France Nuclear Fleet
# =====================================================================
def create_nucleaire():
    w, h = 900, 900
    im = Image.new("RGBA", (w, h), (248, 250, 252, 255))
    draw = ImageDraw.Draw(im)

    # Title header
    draw.rectangle([0, 0, w, 80], fill=(28, 43, 65, 255))
    draw.text((40, 18), "PARC ÉLECTRONUCLÉAIRE FRANÇAIS", font=get_font(28, bold=True), fill=(255, 255, 255, 255))
    draw.text((42, 50), "Répartition géographique des 19 centrales nucléaires de production d'électricité", font=get_font(15), fill=(180, 205, 230, 255))

    # France outline approximate polygon
    france_pts = [
        (400, 140), (470, 150), (580, 200), (660, 250), (620, 320),
        (660, 410), (630, 530), (590, 600), (580, 720), (530, 750),
        (470, 750), (380, 730), (330, 770), (280, 740), (290, 640),
        (250, 550), (280, 470), (200, 370), (140, 350), (170, 300),
        (260, 280), (310, 200), (350, 160)
    ]
    draw.polygon(france_pts, fill=(232, 238, 245, 255), outline=(160, 180, 200, 255), width=3)

    # Rivers (simplified)
    # Seine
    draw.line([(310, 200), (380, 250), (450, 270), (510, 340)], fill=(160, 200, 235, 255), width=3)
    # Loire
    draw.line([(260, 380), (360, 370), (450, 370), (490, 420), (520, 520)], fill=(160, 200, 235, 255), width=3)
    # Rhone
    draw.line([(630, 440), (580, 470), (570, 600), (560, 710)], fill=(160, 200, 235, 255), width=3)
    # Garonne
    draw.line([(280, 530), (350, 620), (430, 700)], fill=(160, 200, 235, 255), width=3)

    plants = [
        ("Gravelines", 390, 145, "5 460 MW (6)", True),
        ("Penly", 410, 180, "2 660 MW (2)", False),
        ("Paluel", 370, 195, "5 320 MW (4)", True),
        ("Flamanville", 280, 220, "2 660 MW (2)", False),
        ("Chooz", 540, 160, "3 000 MW (2)", False),
        ("Cattenom", 610, 230, "5 200 MW (4)", True),
        ("Nogent-sur-Seine", 480, 280, "2 620 MW (2)", False),
        ("Dampierre", 450, 340, "3 560 MW (4)", True),
        ("Saint-Laurent", 410, 360, "1 830 MW (2)", False),
        ("Chinon", 370, 375, "3 620 MW (4)", True),
        ("Belleville", 480, 380, "2 620 MW (2)", False),
        ("Civaux", 390, 440, "2 990 MW (2)", False),
        ("Blayais", 320, 520, "3 640 MW (4)", True),
        ("Golfech", 380, 620, "2 620 MW (2)", False),
        ("Fessenheim", 650, 340, "1 760 MW (2)", False),
        ("Bugey", 590, 460, "3 580 MW (4)", True),
        ("Saint-Alban", 560, 510, "2 670 MW (2)", False),
        ("Cruas", 565, 570, "3 660 MW (4)", True),
        ("Tricastin", 560, 610, "3 660 MW (4)", True),
    ]

    for name, px, py, mw, big in plants:
        rad = 9 if big else 6
        color = (210, 40, 30, 255) if big else (230, 120, 20, 255)
        draw.ellipse([px-rad, py-rad, px+rad, py+rad], fill=color, outline=(255, 255, 255, 255), width=2)
        draw.text((px + rad + 4, py - 8), name, font=get_font(13, bold=True), fill=(20, 30, 45, 255))
        draw.text((px + rad + 4, py + 6), mw, font=get_font(11), fill=(90, 100, 115, 255))

    # Legend box
    draw.rounded_rectangle([40, 770, 420, 870], radius=8, fill=(255, 255, 255, 230), outline=(180, 190, 205, 255), width=2)
    draw.text((55, 780), "LÉGENDE ET CARACTÉRISTIQUES DU PARC", font=get_font(13, bold=True), fill=(20, 30, 50, 255))
    draw.ellipse([60, 810, 76, 826], fill=(210, 40, 30, 255), outline=(255, 255, 255, 255))
    draw.text((85, 810), "Centrale à 4 ou 6 réacteurs (> 3 500 MW)", font=get_font(12), fill=(40, 50, 65, 255))
    draw.ellipse([63, 840, 75, 852], fill=(230, 120, 20, 255), outline=(255, 255, 255, 255))
    draw.text((85, 838), "Centrale à 2 réacteurs (< 3 000 MW)", font=get_font(12), fill=(40, 50, 65, 255))

    im.save(os.path.join(ASSET_DIR, "Nucléaire.png"))
    print("-> Generated Nucléaire.png")

create_nucleaire()

# =====================================================================
# 3. stats.png (571 x 553) - Solar Irradiance Map
# =====================================================================
def create_stats():
    w, h = 571, 553
    im = Image.new("RGBA", (w, h), (248, 250, 252, 255))
    draw = ImageDraw.Draw(im)

    # Title
    draw.rectangle([0, 0, w, 60], fill=(230, 120, 20, 255))
    draw.text((25, 12), "RAYONNEMENT SOLAIRE EN FRANCE", font=get_font(20, bold=True), fill=(255, 255, 255, 255))
    draw.text((26, 36), "Ensoleillement global annuel moyen sur surface inclinée (kWh/m²/an)", font=get_font(12), fill=(255, 240, 220, 255))

    # 4 Color Zones in France shape
    zones = [
        # Zone 4 (North): < 1200 kWh/m2
        ([(250, 80), (380, 80), (450, 150), (350, 220), (160, 210), (210, 120)], (255, 235, 160, 255), "< 1 200 kWh/m²"),
        # Zone 3 (Center): 1200 - 1350
        ([(160, 210), (350, 220), (450, 230), (430, 330), (190, 320), (140, 250)], (255, 210, 90, 255), "1 200 - 1 350 kWh/m²"),
        # Zone 2 (South West/Rhone): 1350 - 1500
        ([(190, 320), (430, 330), (460, 420), (360, 450), (210, 430)], (255, 170, 40, 255), "1 350 - 1 500 kWh/m²"),
        # Zone 1 (Mediterranean/South): > 1500
        ([(210, 430), (360, 450), (460, 420), (440, 500), (330, 510)], (240, 90, 15, 255), "> 1 500 kWh/m² (Sud & Corse)")
    ]

    for pts, col, label in zones:
        draw.polygon(pts, fill=col, outline=(255, 255, 255, 200), width=2)

    # Legend
    draw.rounded_rectangle([25, 430, 260, 530], radius=6, fill=(255, 255, 255, 235), outline=(200, 210, 220, 255), width=2)
    draw.text((35, 435), "LÉGENDE ENERGIE SOLAIRE", font=get_font(12, bold=True), fill=(40, 40, 40, 255))
    ly = 455
    for _, col, label in zones:
        draw.rectangle([35, ly, 55, ly+14], fill=col, outline=(100, 100, 100, 200))
        draw.text((65, ly), label, font=get_font(11), fill=(30, 30, 30, 255))
        ly += 18

    im.save(os.path.join(ASSET_DIR, "stats.png"))
    print("-> Generated stats.png")

create_stats()

# =====================================================================
# 4. potentiel.png (541 x 700) - Wind Potential Map
# =====================================================================
def create_potentiel():
    w, h = 541, 700
    im = Image.new("RGBA", (w, h), (248, 250, 252, 255))
    draw = ImageDraw.Draw(im)

    # Title header
    draw.rectangle([0, 0, w, 70], fill=(20, 85, 140, 255))
    draw.text((25, 14), "GISEMENT ÉOLIEN EN FRANCE", font=get_font(22, bold=True), fill=(255, 255, 255, 255))
    draw.text((26, 42), "Vitesse moyenne du vent à 50 mètres du sol (en m/s)", font=get_font(12), fill=(200, 230, 255, 255))

    # Base shape of France
    france_poly = [
        (250, 100), (360, 120), (450, 180), (410, 300), (450, 450),
        (390, 550), (320, 560), (220, 530), (180, 420), (90, 290),
        (130, 250), (200, 210), (230, 120)
    ]
    draw.polygon(france_poly, fill=(225, 238, 248, 255), outline=(150, 175, 200, 255), width=2)

    # High wind potential coastal and valley belts
    # Manche / Atlantic coast (> 6.5 m/s)
    draw.polygon([(110, 280), (170, 220), (240, 120), (260, 140), (190, 240), (130, 300)], fill=(30, 110, 190, 200))
    # Rhône Valley / Languedoc (Mistral & Tramontane: > 6.0 m/s)
    draw.polygon([(360, 430), (410, 450), (390, 540), (340, 530)], fill=(40, 140, 210, 200))

    # Text callouts
    draw.text((70, 230), "Bretagne &\nCôtes de la Manche\n> 6.5 m/s", font=get_font(12, bold=True), fill=(10, 60, 120, 255))
    draw.text((380, 470), "Vallée du Rhône &\nGolfe du Lion\n> 6.0 m/s", font=get_font(12, bold=True), fill=(10, 60, 120, 255))

    # Legend
    draw.rounded_rectangle([25, 580, 515, 680], radius=8, fill=(255, 255, 255, 240), outline=(180, 200, 220, 255), width=2)
    draw.text((35, 590), "POTENTIEL ÉOLIEN ET FORMULE DE BETZ", font=get_font(13, bold=True), fill=(20, 50, 90, 255))
    draw.text((35, 612), "Limite théorique de Betz : Rendement maximal = 16/27 ≈ 59,3 % de l'énergie du vent.", font=get_font(11), fill=(50, 60, 75, 255))
    draw.text((35, 630), "Formule de calcul : P = 0.5 × ρ × S × v³ × Cp (avec Cp ≈ 0.40 et ρ = 1.23 kg/m³)", font=get_font(11, bold=True), fill=(20, 80, 140, 255))
    draw.text((35, 650), "La production varie comme le CUBE de la vitesse : doubler la vitesse multiplie par 8 l'énergie !", font=get_font(11), fill=(180, 40, 20, 255))

    im.save(os.path.join(ASSET_DIR, "potentiel.png"))
    print("-> Generated potentiel.png")

create_potentiel()

# =====================================================================
# 5. eoliennes_taille.png (985 x 467) - Wind Turbine Evolution Chart
# =====================================================================
def create_eoliennes_taille():
    w, h = 985, 467
    im = Image.new("RGBA", (w, h), (252, 253, 255, 255))
    draw = ImageDraw.Draw(im)

    # Title header
    draw.rectangle([0, 0, w, 65], fill=(35, 95, 70, 255))
    draw.text((35, 12), "ÉVOLUTION DE LA TAILLE ET PUISSANCE DES ÉOLIENNES", font=get_font(24, bold=True), fill=(255, 255, 255, 255))
    draw.text((37, 40), "Comparaison historique des dimensions et de la puissance unitaire nominale", font=get_font(13), fill=(200, 240, 220, 255))

    # Ground line
    ground_y = 390
    draw.line([(30, ground_y), (w-30, ground_y)], fill=(120, 150, 130, 255), width=3)

    # Height ruler on left
    for m in range(0, 181, 30):
        ry = ground_y - int(m * 1.6)
        draw.line([(50, ry), (65, ry)], fill=(150, 160, 170, 255), width=2)
        draw.text((72, ry-7), f"{m} m", font=get_font(11), fill=(100, 110, 120, 255))

    turbines = [
        ("Années 1990", 220, 40, 30, "300 kW", (120, 140, 130, 255)),
        ("Années 2000", 420, 75, 60, "1.5 MW", (90, 160, 120, 255)),
        ("Années 2010", 640, 115, 95, "3.0 MW", (40, 140, 90, 255)),
        ("Aujourd'hui (2020+)", 850, 155, 140, "5 - 8 MW", (20, 110, 60, 255))
    ]

    for label, cx, hub_h, rotor_d, power, col in turbines:
        top_y = ground_y - int(hub_h * 1.6)
        rotor_r = int((rotor_d / 2) * 1.6)
        # Mast
        draw.polygon([(cx-3, top_y), (cx+3, top_y), (cx+7, ground_y), (cx-7, ground_y)], fill=col)
        # Rotor circle (dashed/translucent)
        draw.ellipse([cx-rotor_r, top_y-rotor_r, cx+rotor_r, top_y+rotor_r], outline=(col[0], col[1], col[2], 120), width=2)
        # 3 Blades
        for a in [0, 120, 240]:
            rad = math.radians(a)
            bx = cx + math.sin(rad) * rotor_r
            by = top_y - math.cos(rad) * rotor_r
            draw.line([(cx, top_y), (bx, by)], fill=col, width=3)
        # Hub
        draw.ellipse([cx-5, top_y-5, cx+5, top_y+5], fill=(50, 50, 50, 255))
        # Labels
        draw.text((cx - 45, ground_y + 10), label, font=get_font(13, bold=True), fill=(30, 40, 50, 255))
        draw.text((cx - 35, ground_y + 30), f"Rotor: {rotor_d}m", font=get_font(11), fill=(70, 80, 90, 255))
        draw.text((cx - 30, ground_y + 45), f"P: {power}", font=get_font(12, bold=True), fill=col)

    im.save(os.path.join(ASSET_DIR, "eoliennes_taille.png"))
    print("-> Generated eoliennes_taille.png")

create_eoliennes_taille()

# =====================================================================
# 6. Eo_puiss.png (450 x 337) - Wind Turbine Power Curve
# =====================================================================
def create_eo_puiss():
    w, h = 450, 337
    im = Image.new("RGBA", (w, h), (255, 255, 255, 255))
    draw = ImageDraw.Draw(im)

    # Title
    draw.rectangle([0, 0, w, 40], fill=(20, 100, 80, 255))
    draw.text((20, 10), "COURBE DE PUISSANCE D'UNE ÉOLIENNE", font=get_font(14, bold=True), fill=(255, 255, 255, 255))

    # Axes
    ox, oy = 55, 280
    draw.line([(ox, oy), (ox+360, oy)], fill=(50, 50, 50, 255), width=2)
    draw.line([(ox, oy), (ox, oy-210)], fill=(50, 50, 50, 255), width=2)

    draw.text((ox+120, oy+20), "Vitesse du vent (m/s)", font=get_font(11), fill=(40, 40, 40, 255))
    draw.text((10, oy-225), "Puissance (%)", font=get_font(11), fill=(40, 40, 40, 255))

    # Curve points
    curve = []
    # 0 to 3 m/s: 0 power
    for v in range(0, 4):
        x = ox + v * 12
        curve.append((x, oy))
    # 3 to 12 m/s: cubic rise
    for v in range(3, 13):
        x = ox + v * 12
        norm = (v - 3) / 9.0
        p = (norm ** 2.7) * 190
        curve.append((x, int(oy - p)))
    # 12 to 25 m/s: flat rated power
    for v in range(12, 26):
        x = ox + v * 12
        curve.append((x, oy - 190))
    # 25 m/s: cut-out drop
    curve.append((ox + 25 * 12, oy))
    curve.append((ox + 28 * 12, oy))

    draw.line(curve, fill=(20, 140, 90, 255), width=3)

    # Annotations
    draw.text((ox+38, oy-40), "Démarrage\n(3 m/s)", font=get_font(10), fill=(100, 110, 120, 255))
    draw.text((ox+150, oy-210), "Puissance nominale (100%)", font=get_font(10, bold=True), fill=(20, 120, 70, 255))
    draw.text((ox+295, oy-40), "Coupure\n(25 m/s)", font=get_font(10), fill=(180, 40, 30, 255))

    im.save(os.path.join(ASSET_DIR, "Eo_puiss.png"))
    # Also save as gif for compatibility
    im.convert("RGB").save(os.path.join(ASSET_DIR, "Eo_puiss.gif"))
    print("-> Generated Eo_puiss.png and Eo_puiss.gif")

create_eo_puiss()

# =====================================================================
# 7. credits3.png (878 x 572) - Clean Modern Credits Screen
# =====================================================================
def create_credits3():
    w, h = 878, 572
    im = Image.new("RGBA", (w, h), (18, 26, 38, 255))
    draw = ImageDraw.Draw(im)

    # Background subtle radial gradient & glow
    for r in range(350, 0, -20):
        draw.ellipse([w//2-r, h//2-r, w//2+r, h//2+r], fill=(25, 45, 75, int(120 * (1 - r/380))))

    # Title & Subtitle
    draw.text((w//2 - 290, 60), "TPE - SIMULATEUR D'ÉNERGIES", font=get_font(34, bold=True), fill=(255, 255, 255, 255))
    draw.text((w//2 - 210, 115), "Énergies Renouvelables vs Nucléaire", font=get_font(20), fill=(120, 200, 240, 255))

    # Divider line
    draw.line([(w//2 - 240, 155), (w//2 + 240, 155)], fill=(60, 120, 180, 200), width=2)

    # Credits text
    lines = [
        ("Développement initial & modélisation :", "Rémi Fache (2014-2015)"),
        ("Mise à jour, modernisation & portabilité :", "Version 1.5 (2026)"),
        ("Technologies :", "C++17, Qt Widgets, SDL 1.2, CMake"),
        ("Systèmes compatibles :", "GNU/Linux, Microsoft Windows"),
        ("Ressources graphiques :", "100% Libres de droits (CC0 / Open Source)"),
        ("Polices de caractères :", "Droid Sans (Apache 2.0) / Patrick Hand (OFL)"),
    ]

    cy = 190
    for title, val in lines:
        draw.text((w//2 - 280, cy), title, font=get_font(15, bold=True), fill=(200, 220, 240, 255))
        draw.text((w//2 + 70, cy), val, font=get_font(15), fill=(255, 255, 255, 255))
        cy += 40

    # Footer banner
    draw.rounded_rectangle([w//2 - 200, 460, w//2 + 200, 520], radius=10, fill=(30, 55, 90, 255), outline=(70, 140, 220, 255), width=2)
    draw.text((w//2 - 120, 480), "© Rémi Fache - Projet TPE", font=get_font(18, bold=True), fill=(255, 255, 255, 255))

    im.save(os.path.join(ASSET_DIR, "credits3.png"))
    # Also save as credits.png for SDL
    im.save(os.path.join(ASSET_DIR, "credits.png"))
    print("-> Generated credits3.png and credits.png")

create_credits3()

# =====================================================================
# 8. Icons: icone.png, icone.ico, and Toolbar / Menu Icons
# =====================================================================
def create_icons():
    # 1. Main App Icon (512 x 512)
    im = Image.new("RGBA", (512, 512), (0, 0, 0, 0))
    draw = ImageDraw.Draw(im)

    # Rounded outer shield
    draw.rounded_rectangle([30, 30, 482, 482], radius=100, fill=(24, 38, 58, 255), outline=(70, 150, 230, 255), width=12)

    # Atom orbits (Nuclear)
    draw.ellipse([120, 200, 392, 312], outline=(230, 120, 30, 220), width=8)
    draw.ellipse([200, 120, 312, 392], outline=(50, 160, 240, 220), width=8)

    # Central Core / Sun
    draw.ellipse([226, 226, 286, 286], fill=(255, 220, 40, 255))

    # Wind turbine blade accent in green
    draw.line([(256, 226), (256, 90)], fill=(40, 200, 90, 255), width=10)
    draw.line([(256, 286), (360, 380)], fill=(40, 200, 90, 255), width=10)
    draw.line([(226, 256), (120, 340)], fill=(40, 200, 90, 255), width=10)

    im.save(os.path.join(ASSET_DIR, "icone.png"))
    im.save(os.path.join("assets", "icone.png"))

    # Generate multi-size ICO
    im.save(os.path.join(ASSET_DIR, "icone.ico"), format="ICO", sizes=[(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
    im.save(os.path.join("assets", "icone.ico"), format="ICO", sizes=[(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
    print("-> Generated icone.png & icone.ico")

    # 2. fileopen.png (32 x 32)
    fo = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    fod = ImageDraw.Draw(fo)
    fod.rounded_rectangle([2, 8, 28, 28], radius=3, fill=(230, 190, 70, 255), outline=(170, 130, 30, 255))
    fod.polygon([(2, 14), (28, 14), (25, 28), (5, 28)], fill=(255, 220, 100, 255))
    fo.save(os.path.join(ASSET_DIR, "fileopen.png"))

    # 3. icone_save.png (32 x 32)
    fs = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    fsd = ImageDraw.Draw(fs)
    fsd.rounded_rectangle([3, 3, 29, 29], radius=3, fill=(40, 100, 180, 255), outline=(20, 60, 120, 255))
    fsd.rectangle([7, 3, 25, 14], fill=(240, 240, 240, 255))
    fsd.rectangle([9, 18, 23, 27], fill=(220, 220, 220, 255))
    fs.save(os.path.join(ASSET_DIR, "icone_save.png"))

    # 4. icone_quit.png (32 x 32)
    fq = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    fqd = ImageDraw.Draw(fq)
    fqd.ellipse([3, 3, 29, 29], fill=(210, 50, 40, 255), outline=(150, 30, 20, 255))
    fqd.line([(10, 10), (22, 22)], fill=(255, 255, 255, 255), width=3)
    fqd.line([(22, 10), (10, 22)], fill=(255, 255, 255, 255), width=3)
    fq.save(os.path.join(ASSET_DIR, "icone_quit.png"))

    # 5. eolienne_fixe.png (48 x 48)
    ef = Image.new("RGBA", (48, 48), (0, 0, 0, 0))
    efd = ImageDraw.Draw(ef)
    efd.polygon([(22, 24), (26, 24), (27, 44), (21, 44)], fill=(120, 130, 140, 255))
    for a in [0, 120, 240]:
        rad = math.radians(a)
        efd.line([(24, 24), (24 + math.sin(rad)*18, 24 - math.cos(rad)*18)], fill=(30, 160, 80, 255), width=3)
    efd.ellipse([21, 21, 27, 27], fill=(40, 50, 60, 255))
    ef.save(os.path.join(ASSET_DIR, "eolienne_fixe.png"))

    # 6. Photo.png (48 x 48)
    fp = Image.new("RGBA", (48, 48), (0, 0, 0, 0))
    fpd = ImageDraw.Draw(fp)
    fpd.polygon([(6, 38), (42, 38), (36, 14), (12, 14)], fill=(20, 60, 140, 255), outline=(180, 200, 230, 255), width=2)
    fpd.line([(24, 14), (24, 38)], fill=(140, 180, 230, 255))
    fpd.line([(9, 26), (39, 26)], fill=(140, 180, 230, 255))
    fp.save(os.path.join(ASSET_DIR, "Photo.png"))

    # 7. nuclo.png & nuclo1.jpg (48 x 48)
    fn = Image.new("RGBA", (48, 48), (0, 0, 0, 0))
    fnd = ImageDraw.Draw(fn)
    fnd.polygon([(8, 44), (40, 44), (34, 16), (36, 8), (12, 8), (14, 16)], fill=(200, 205, 215, 255), outline=(100, 105, 115, 255))
    fnd.ellipse([18, 20, 30, 32], fill=(230, 120, 20, 255))
    fn.save(os.path.join(ASSET_DIR, "nuclo.png"))
    fn.convert("RGB").save(os.path.join(ASSET_DIR, "nuclo1.jpg"))

    # 8. pico.png (32 x 32)
    pi = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    pid = ImageDraw.Draw(pi)
    pid.rounded_rectangle([4, 4, 28, 28], radius=4, fill=(30, 140, 220, 255), outline=(20, 80, 160, 255))
    pid.text((8, 6), "PV", font=get_font(12, bold=True), fill=(255, 255, 255, 255))
    pi.save(os.path.join(ASSET_DIR, "pico.png"))

    print("-> Generated all toolbar icons")

create_icons()

# =====================================================================
# 9. SDL Presentation: FOND.png, robot.bmp, retour.bmp, bulle1..6, eolienne.gif
# =====================================================================
def create_sdl_assets():
    # 1. FOND.png (1040 x 832)
    fond = Image.new("RGBA", (1040, 832), (24, 32, 48, 255))
    fd = ImageDraw.Draw(fond)
    # Background styling with grid & banner
    fd.rectangle([0, 0, 1040, 120], fill=(16, 22, 34, 255))
    fd.text((1040//2 - 280, 35), "TPE - PRÉSENTATION DES ÉNERGIES", font=get_font(30, bold=True), fill=(255, 255, 255, 255))

    # Decorative buttons areas (matching posiclic coords in sdl1.cpp)
    btn1 = [315, 215, 727, 287] # Presentation generale
    btn2 = [315, 350, 735, 410] # Themes / Simulation
    btn3 = [315, 474, 730, 542] # Credits

    fd.rounded_rectangle(btn1, radius=12, fill=(35, 110, 180, 255), outline=(70, 160, 240, 255), width=3)
    fd.text((btn1[0]+45, btn1[1]+22), "1. Présentation Générale", font=get_font(22, bold=True), fill=(255, 255, 255, 255))

    fd.rounded_rectangle(btn2, radius=12, fill=(30, 140, 90, 255), outline=(60, 200, 140, 255), width=3)
    fd.text((btn2[0]+55, btn2[1]+22), "2. Simulateur Complet", font=get_font(22, bold=True), fill=(255, 255, 255, 255))

    fd.rounded_rectangle(btn3, radius=12, fill=(130, 70, 160, 255), outline=(180, 110, 220, 255), width=3)
    fd.text((btn3[0]+120, btn3[1]+22), "3. Crédits", font=get_font(22, bold=True), fill=(255, 255, 255, 255))

    fond.save(os.path.join(ASSET_DIR, "FOND.png"))

    # 2. robot.bmp (200 x 200) - Friendly original robot mascot "Volt"
    robot = Image.new("RGB", (200, 200), (255, 255, 255))
    rd = ImageDraw.Draw(robot)
    # Head
    rd.rounded_rectangle([50, 30, 150, 100], radius=16, fill=(180, 195, 210), outline=(50, 60, 75), width=3)
    # Antenna
    rd.line([(100, 30), (100, 12)], fill=(50, 60, 75), width=3)
    rd.ellipse([93, 5, 107, 19], fill=(240, 80, 60))
    # Eyes & Screen
    rd.rounded_rectangle([65, 45, 135, 85], radius=8, fill=(30, 40, 50))
    rd.ellipse([75, 55, 90, 75], fill=(50, 210, 240))
    rd.ellipse([110, 55, 125, 75], fill=(50, 210, 240))
    # Body
    rd.rounded_rectangle([45, 110, 155, 185], radius=14, fill=(150, 170, 190), outline=(50, 60, 75), width=3)
    # Chest meter / energy badge
    rd.ellipse([80, 125, 120, 165], fill=(255, 215, 50), outline=(50, 60, 75), width=2)
    rd.polygon([(100, 130), (92, 146), (103, 146), (97, 160), (108, 142), (98, 142)], fill=(220, 40, 20))
    robot.save(os.path.join(ASSET_DIR, "robot.bmp"))

    # 3. retour.bmp (68 x 68) - Back arrow
    ret = Image.new("RGB", (68, 68), (0, 255, 0)) # Green colorkey
    retd = ImageDraw.Draw(ret)
    retd.rounded_rectangle([4, 4, 64, 64], radius=10, fill=(30, 130, 210), outline=(10, 70, 140), width=2)
    # White Arrow left
    retd.polygon([(15, 34), (35, 18), (35, 26), (52, 26), (52, 42), (35, 42), (35, 50)], fill=(255, 255, 255))
    ret.save(os.path.join(ASSET_DIR, "retour.bmp"))

    # 4. bulle_vierge.png & bulle1..6.png (360 x 194)
    bubble_texts = [
        "Bonjour ! Je suis Volt, votre guide.\nBienvenue dans cette présentation sur les\nénergies renouvelables et le nucléaire !",
        "Comment équilibrer la production\nélectrique face à l'urgence climatique ?\nDécouvrons les piliers du mix énergétique.",
        "L'énergie éolienne convertit l'énergie\ndu vent. Selon la limite de Betz, son\nrendement maximal théorique est de 59.3%.",
        "Le solaire photovoltaïque capte les\nphotons du soleil pour générer du courant\ngrâce à l'effet photoélectrique du silicium.",
        "L'énergie nucléaire utilise la fission de\nl'uranium : une forte densité énergétique,\npilotable en continu et sans émission directe.",
        "Prêt à simuler ? Cliquez sur la flèche pour\naccéder au simulateur et tester vos propres\nscénarios de production annuelle !"
    ]

    for idx, txt in enumerate(bubble_texts, 1):
        bim = Image.new("RGBA", (360, 194), (0, 0, 0, 0))
        bd = ImageDraw.Draw(bim)
        # Bubble body
        bd.rounded_rectangle([10, 10, 350, 170], radius=18, fill=(245, 250, 255, 245), outline=(40, 110, 180, 255), width=3)
        # Pointer at bottom
        bd.polygon([(40, 168), (65, 168), (45, 190)], fill=(245, 250, 255, 245))
        bd.line([(40, 168), (45, 190)], fill=(40, 110, 180, 255), width=3)
        bd.line([(65, 168), (45, 190)], fill=(40, 110, 180, 255), width=3)

        # Text inside
        fnt = get_font(13, bold=False)
        bd.text((25, 35), txt, font=fnt, fill=(20, 35, 55, 255), spacing=6)

        bim.save(os.path.join(ASSET_DIR, f"bulle{idx}.png"))
        if idx == 1:
            # Also save blank bubble
            b_blank = Image.new("RGBA", (360, 194), (0, 0, 0, 0))
            bbd = ImageDraw.Draw(b_blank)
            bbd.rounded_rectangle([10, 10, 350, 170], radius=18, fill=(245, 250, 255, 245), outline=(40, 110, 180, 255), width=3)
            bbd.polygon([(40, 168), (65, 168), (45, 190)], fill=(245, 250, 255, 245))
            bbd.line([(40, 168), (45, 190)], fill=(40, 110, 180, 255), width=3)
            bbd.line([(65, 168), (45, 190)], fill=(40, 110, 180, 255), width=3)
            b_blank.save(os.path.join(ASSET_DIR, "bulle_vierge.png"))

    # 5. eolienne.gif (152 x 114) - Animated rotating wind turbine
    frames = []
    for f in range(8):
        f_im = Image.new("RGBA", (152, 114), (255, 255, 255, 0))
        fd = ImageDraw.Draw(f_im)
        # Mast
        cx, cy = 76, 50
        fd.polygon([(cx-2, cy), (cx+2, cy), (cx+4, 105), (cx-4, 105)], fill=(180, 190, 200, 255))
        # Blades rotated by f * 45 / 3 = 15 deg
        base_angle = f * 15
        for b_idx in [0, 120, 240]:
            rad = math.radians(base_angle + b_idx)
            bx = cx + math.sin(rad) * 36
            by = cy - math.cos(rad) * 36
            fd.line([(cx, cy), (bx, by)], fill=(40, 150, 90, 255), width=3)
        fd.ellipse([cx-4, cy-4, cx+4, cy+4], fill=(60, 70, 80, 255))
        frames.append(f_im.convert("RGB"))

    frames[0].save(os.path.join(ASSET_DIR, "eolienne.gif"), save_all=True, append_images=frames[1:], duration=100, loop=0)
    print("-> Generated all SDL assets")

create_sdl_assets()
print("All assets generated successfully!")
