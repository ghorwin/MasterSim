#!/usr/bin/env python3
"""
Erzeugt die SVG-Abbildungen fuer doc/algorithm/CoSim_InterpolatingOutputs.md.

Keine externen Abhaengigkeiten (nur Python-Standardbibliothek).

Alle Abbildungen verwenden denselben Massstab (1 SVG-Einheit = 0,2 mm), damit
Schriftgroessen und Linienstaerken im Druck einheitlich sind:
  Grundschrift 14 Einheiten = 2,8 mm (ca. 8 pt), Annotationen 12 Einheiten.
Maximale Breite 800 Einheiten = 160 mm (Satzspiegel).

Aufruf (aus beliebigem Verzeichnis):
    python3 doc/algorithm/figures/generate_figures.py
"""

import csv
import math
import os

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
TESTDIR = os.path.join(REPO, "data", "tests", "linux64", "Room_Radiator")

MM_PER_UNIT = 0.2

# --- Stil -------------------------------------------------------------------

FONT = "Liberation Sans, Arial, Helvetica, sans-serif"
MONO = "Liberation Mono, DejaVu Sans Mono, Consolas, monospace"

INK = "#1a1a1a"
GRAY = "#6b6b6b"
LIGHTGRAY = "#bdbdbd"
PANEL = "#f4f4f4"
ACCENT = "#1f5fa8"         # Erzeuger / Ableitungsinformation
ACCENT_FILL = "#e5edf7"
ORANGE = "#c0601a"         # Verbraucher
ORANGE_FILL = "#fbeadc"
ERROR_FILL = "#f2c9a8"
RED = "#b2282e"
WHITE = "#ffffff"

COLORS = {"ink": INK, "gray": GRAY, "accent": ACCENT, "orange": ORANGE, "red": RED}


def esc(s):
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def f(v):
    """Zahl kompakt formatieren."""
    return ("%.2f" % v).rstrip("0").rstrip(".")


# --- Rich text: `code`, x_{sub}, x^{sup}, x_n --------------------------------

def parse_rich(s):
    tokens = []
    buf = ""
    i = 0
    while i < len(s):
        c = s[i]
        if c == "`":
            j = s.index("`", i + 1)
            if buf:
                tokens.append(("n", buf))
                buf = ""
            tokens.append(("mono", s[i + 1:j]))
            i = j + 1
            continue
        if c in "_^" and i + 1 < len(s):
            if s[i + 1] == "{":
                depth = 0
                j = i + 1
                while True:
                    if s[j] == "{":
                        depth += 1
                    elif s[j] == "}":
                        depth -= 1
                        if depth == 0:
                            break
                    j += 1
                content = s[i + 2:j]
                i = j + 1
            else:
                content = s[i + 1]
                i += 2
            if buf:
                tokens.append(("n", buf))
                buf = ""
            tokens.append(("sub" if c == "_" else "sup", content))
            continue
        buf += c
        i += 1
    if buf:
        tokens.append(("n", buf))
    return tokens


def rich_tspans(s, size):
    out = []
    pending = 0.0
    for kind, txt in parse_rich(s):
        attrs = ""
        dy = pending
        pending = 0.0
        if kind == "mono":
            attrs = ' font-family="%s" font-size="%s"' % (MONO, f(size * 0.92))
        elif kind == "sub":
            dy += 0.28 * size
            pending = -0.28 * size
            attrs = ' font-size="%s"' % f(size * 0.72)
        elif kind == "sup":
            dy -= 0.38 * size
            pending = 0.38 * size
            attrs = ' font-size="%s"' % f(size * 0.72)
        if dy != 0.0:
            attrs += ' dy="%s"' % f(dy)
        out.append("<tspan%s>%s</tspan>" % (attrs, esc(txt)))
    return "".join(out)


class Fig:
    def __init__(self, w, h):
        self.w = w
        self.h = h
        self.items = []

    # -- Grundelemente --
    def add(self, s):
        self.items.append(s)

    def text(self, x, y, s, size=14, anchor="start", weight=None, fill=INK,
             italic=False, halo=False, rotate=None):
        lines = s if isinstance(s, list) else [s]
        lh = 1.28 * size
        for k, line in enumerate(lines):
            attrs = 'x="%s" y="%s" font-size="%s" text-anchor="%s" fill="%s"' % (
                f(x), f(y + k * lh), f(size), anchor, fill)
            if weight:
                attrs += ' font-weight="%s"' % weight
            if italic:
                attrs += ' font-style="italic"'
            if rotate is not None:
                attrs += ' transform="rotate(%s %s %s)"' % (f(rotate), f(x), f(y))
            if halo:
                self.add('<text %s stroke="%s" stroke-width="5" stroke-linejoin="round">%s</text>'
                         % (attrs.replace('fill="%s"' % fill, 'fill="%s"' % WHITE), WHITE,
                            rich_tspans(line, size)))
            self.add("<text %s>%s</text>" % (attrs, rich_tspans(line, size)))

    def rect(self, x, y, w, h, fill=WHITE, stroke=INK, sw=1.2, rx=4, dash=None):
        d = ' stroke-dasharray="%s"' % dash if dash else ""
        self.add('<rect x="%s" y="%s" width="%s" height="%s" rx="%s" fill="%s" stroke="%s" stroke-width="%s"%s/>'
                 % (f(x), f(y), f(w), f(h), f(rx), fill, stroke, f(sw), d))

    def box(self, x, y, w, h, lines, fill=WHITE, stroke=INK, size=14, bold_first=False,
            rx=4, sw=1.2, anchor="middle", dash=None, line_colors=None):
        self.rect(x, y, w, h, fill, stroke, sw, rx, dash)
        lines = lines if isinstance(lines, list) else [lines]
        lh = 1.28 * size
        y0 = y + h / 2 - (len(lines) - 1) * lh / 2 + 0.36 * size
        tx = x + w / 2 if anchor == "middle" else x + 10
        for k, line in enumerate(lines):
            col = line_colors[k] if line_colors else INK
            self.text(tx, y0 + k * lh, line, size=size, anchor=anchor,
                      weight="bold" if (bold_first and k == 0) else None, fill=col)

    def line(self, pts, color=INK, sw=1.4, dash=None, end=True, start=False, fill="none"):
        d = "M" + " L".join("%s,%s" % (f(px), f(py)) for px, py in pts)
        self.path(d, color, sw, dash, end, start, fill)

    def path(self, d, color=INK, sw=1.4, dash=None, end=False, start=False, fill="none"):
        key = [k for k, v in COLORS.items() if v == color]
        key = key[0] if key else "ink"
        attrs = 'd="%s" fill="%s" stroke="%s" stroke-width="%s" stroke-linejoin="round"' % (d, fill, color, f(sw))
        if dash:
            attrs += ' stroke-dasharray="%s"' % dash
        if end:
            attrs += ' marker-end="url(#arr-%s)"' % key
        if start:
            attrs += ' marker-start="url(#arrs-%s)"' % key
        self.add("<path %s/>" % attrs)

    def polyline(self, pts, color=INK, sw=1.6, dash=None):
        self.line(pts, color, sw, dash, end=False)

    def polygon(self, pts, fill, stroke="none", sw=0, opacity=1.0):
        self.add('<polygon points="%s" fill="%s" stroke="%s" stroke-width="%s" fill-opacity="%s"/>'
                 % (" ".join("%s,%s" % (f(a), f(b)) for a, b in pts), fill, stroke, f(sw), f(opacity)))

    def circle(self, x, y, r, fill=INK, stroke=None, sw=1.2):
        st = ' stroke="%s" stroke-width="%s"' % (stroke, f(sw)) if stroke else ""
        self.add('<circle cx="%s" cy="%s" r="%s" fill="%s"%s/>' % (f(x), f(y), f(r), fill, st))

    def diamond(self, cx, cy, w, h, lines, fill=WHITE, size=13):
        self.polygon([(cx, cy - h / 2), (cx + w / 2, cy), (cx, cy + h / 2), (cx - w / 2, cy)],
                     fill, INK, 1.2)
        lines = lines if isinstance(lines, list) else [lines]
        lh = 1.28 * size
        y0 = cy - (len(lines) - 1) * lh / 2 + 0.36 * size
        for k, line in enumerate(lines):
            self.text(cx, y0 + k * lh, line, size=size, anchor="middle")

    # -- Ausgabe --
    def save(self, name, title):
        markers = []
        for key, col in COLORS.items():
            markers.append(
                '<marker id="arr-%s" viewBox="0 0 10 8" refX="9.5" refY="4" markerWidth="10" markerHeight="8" '
                'markerUnits="userSpaceOnUse" orient="auto"><path d="M0,0 L10,4 L0,8 z" fill="%s"/></marker>'
                % (key, col))
            markers.append(
                '<marker id="arrs-%s" viewBox="0 0 10 8" refX="0.5" refY="4" markerWidth="10" markerHeight="8" '
                'markerUnits="userSpaceOnUse" orient="auto"><path d="M10,0 L0,4 L10,8 z" fill="%s"/></marker>'
                % (key, col))
        svg = ['<?xml version="1.0" encoding="UTF-8"?>',
               '<svg xmlns="http://www.w3.org/2000/svg" width="%smm" height="%smm" viewBox="0 0 %s %s" '
               'font-family="%s">' % (f(self.w * MM_PER_UNIT), f(self.h * MM_PER_UNIT), f(self.w), f(self.h), FONT),
               "<title>%s</title>" % esc(title),
               "<defs>%s</defs>" % "".join(markers),
               '<rect width="100%" height="100%" fill="#ffffff"/>']
        svg += self.items
        svg.append("</svg>")
        with open(os.path.join(HERE, name), "w", encoding="utf-8") as fh:
            fh.write("\n".join(svg) + "\n")
        print("geschrieben:", name)


# --- Achsen-Helfer -----------------------------------------------------------

class Axes:
    def __init__(self, fig, x0, y0, x1, y1, xr, yr):
        self.fig = fig
        self.x0, self.y0, self.x1, self.y1 = x0, y0, x1, y1   # y0 = oben, y1 = unten
        self.xr, self.yr = xr, yr

    def X(self, v):
        return self.x0 + (v - self.xr[0]) / (self.xr[1] - self.xr[0]) * (self.x1 - self.x0)

    def Y(self, v):
        return self.y1 - (v - self.yr[0]) / (self.yr[1] - self.yr[0]) * (self.y1 - self.y0)

    def P(self, a, b):
        return (self.X(a), self.Y(b))

    def arrows(self, xlabel=None, ylabel=None):
        fg = self.fig
        fg.line([(self.x0, self.y1), (self.x1 + 12, self.y1)], INK, 1.2)
        fg.line([(self.x0, self.y1), (self.x0, self.y0 - 12)], INK, 1.2)
        if xlabel:
            fg.text(self.x1 + 14, self.y1 + 16, xlabel, size=13, anchor="end", italic=True)
        if ylabel:
            fg.text(self.x0 + 6, self.y0 - 4, ylabel, size=13, italic=True)

    def frame(self, xticks, yticks, xfmt, yfmt, grid=True):
        fg = self.fig
        for v in yticks:
            y = self.Y(v)
            if grid:
                fg.polyline([(self.x0, y), (self.x1, y)], "#e3e3e3", 0.8)
            fg.polyline([(self.x0 - 4, y), (self.x0, y)], INK, 1.0)
            fg.text(self.x0 - 7, y + 4, yfmt(v), size=12, anchor="end")
        for v in xticks:
            x = self.X(v)
            if grid:
                fg.polyline([(x, self.y0), (x, self.y1)], "#e3e3e3", 0.8)
            fg.polyline([(x, self.y1), (x, self.y1 + 4)], INK, 1.0)
            fg.text(x, self.y1 + 18, xfmt(v), size=12, anchor="middle")
        fg.rect(self.x0, self.y0, self.x1 - self.x0, self.y1 - self.y0, "none", INK, 1.0, 0)


def de_num(v):
    """Ganzzahl mit Tausenderleerzeichen (deutsche Typografie)."""
    s = "%d" % round(v)
    if len(s) > 4:
        s = s[:-3] + " " + s[-3:]
    return s


# =============================================================================
# Abb. 1: ZOH, Extrapolation, Interpolation
# =============================================================================

def fig_hold_schemes():
    fg = Fig(800, 250)

    def u(t):
        return 0.12 + 0.32 * t - 0.038 * t * t

    def du(t):
        return 0.32 - 0.076 * t

    panels = [
        ("(a) Zero-Order-Hold", "u(t) = u(t_n)", "zoh"),
        ("(b) Taylor-Extrapolation", "u(t) = u(t_n) + du/dt(t_n)·(t − t_n)", "ext"),
        ("(c) Sekanten-Interpolation", "Gerade durch u(t_n) und u(t_{n+1})", "sec"),
    ]
    for p, (title, sub, mode) in enumerate(panels):
        px = 12 + p * 265
        fg.text(px, 18, title, size=14, weight="bold")
        fg.text(px, 36, sub, size=12, fill=GRAY)
        ax = Axes(fg, px + 22, 58, px + 238, 182, (0, 4.3), (0, 1.0))
        # Fehlerflaechen und Naeherung
        for n in range(4):
            ts = [n + k / 20.0 for k in range(21)]
            if mode == "zoh":
                appr = [u(n) for t in ts]
            elif mode == "ext":
                appr = [u(n) + du(n) * (t - n) for t in ts]
            else:
                appr = [u(n) + (u(n + 1) - u(n)) * (t - n) for t in ts]
            poly = [ax.P(t, u(t)) for t in ts] + [ax.P(t, a) for t, a in reversed(list(zip(ts, appr)))]
            fg.polygon(poly, ERROR_FILL, opacity=0.9)
            fg.polyline([ax.P(t, a) for t, a in zip(ts, appr)], ACCENT, 2.2)
            if mode == "zoh" and n > 0:
                fg.polyline([ax.P(n, u(n - 1)), ax.P(n, u(n))], ACCENT, 1.0, dash="2,2")
        fg.polyline([ax.P(k / 50.0, u(k / 50.0)) for k in range(0, 211)], INK, 1.5)
        for n in range(5):
            fg.circle(*ax.P(n, u(n)), r=3.2)
            fg.text(ax.X(n), ax.y1 + 17, "t_%d" % n, size=12, anchor="middle", italic=True)
            fg.polyline([(ax.X(n), ax.y1), (ax.X(n), ax.y1 + 4)], INK, 1.0)
        ax.arrows("t", "u")

    # Legende
    ly = 232
    fg.polyline([(20, ly - 4), (48, ly - 4)], INK, 1.5)
    fg.text(54, ly, "wahrer Verlauf des Eingangs", size=12)
    fg.polyline([(250, ly - 4), (278, ly - 4)], ACCENT, 2.2)
    fg.text(284, ly, "von der FMU angenommener Verlauf", size=12)
    fg.rect(500, ly - 11, 22, 12, ERROR_FILL, "none", 0, 0)
    fg.text(528, ly, "Fehler", size=12)
    fg.circle(600, ly - 4, 3.2)
    fg.text(610, ly, "Werte an Kommunikationspunkten", size=12)
    fg.save("fig01_halteverfahren.svg", "Zero-Order-Hold, Extrapolation und Interpolation")


# =============================================================================
# Abb. 2: Datenfluss Erzeuger -> Master -> Verbraucher
# =============================================================================

def fig_dataflow():
    fg = Fig(640, 500)
    # FMU A
    fg.box(50, 15, 250, 72, ["FMU A (Erzeuger)", "Ausgang y und Ableitungen d^{k}y/dt^{k}",
                             "k ≤ `maxOutputDerivativeOrder`"],
           fill=ACCENT_FILL, stroke=ACCENT, bold_first=True, size=13)
    # Master
    fg.rect(20, 150, 600, 180, PANEL, GRAY, 1.0, 6)
    fg.text(606, 172, "Co-Simulations-Master", size=14, weight="bold", fill=GRAY, anchor="end")
    fg.box(50, 190, 250, 120, ["Variablenvektoren", "Werte: `realyt`, `realytNext`",
                               "Ableitungen: `realDerivyt`,", "`realDerivytNext`"],
           bold_first=True, size=13)
    fg.box(340, 190, 250, 120, ["Verbindungs-Mapping", "u = c_0 + c_1·y",
                                "du/dt = c_1·dy/dt", "(Offset c_0 entfällt)"],
           bold_first=True, size=13, line_colors=[INK, INK, ACCENT, GRAY])
    # FMU B
    fg.box(340, 405, 250, 72, ["FMU B (Verbraucher)", "`canInterpolateInputs=\"true\"`",
                               "Polynom u(t) für t ∈ [t_n, t_n + h]"],
           fill=ORANGE_FILL, stroke=ORANGE, bold_first=True, size=13)

    # A -> Speicher
    fg.line([(150, 87), (150, 188)], INK, 1.5)
    fg.line([(195, 87), (195, 188)], ACCENT, 1.5, dash="6,4")
    fg.text(208, 113, "`fmi2GetReal`(y)", size=13)
    fg.text(208, 133, "`fmi2GetRealOutputDerivatives`(y, k)", size=13, fill=ACCENT)
    # Speicher -> Mapping
    fg.line([(300, 232), (338, 232)], INK, 1.5)
    fg.line([(300, 268), (338, 268)], ACCENT, 1.5, dash="6,4")
    # Mapping -> B
    fg.line([(420, 310), (420, 403)], INK, 1.5)
    fg.line([(465, 310), (465, 403)], ACCENT, 1.5, dash="6,4")
    fg.text(405, 355, "`fmi2SetReal`(u)", size=13, anchor="end")
    fg.text(405, 375, "`fmi2SetRealInputDerivatives`(u, k)", size=13, anchor="end", fill=ACCENT)

    # Legende
    fg.polyline([(30, 432), (70, 432)], INK, 1.5)
    fg.text(78, 436, "Wert (Ordnung 0)", size=12)
    fg.polyline([(30, 456), (70, 456)], ACCENT, 1.5, dash="6,4")
    fg.text(78, 460, "Ableitung (Ordnung ≥ 1)", size=12, fill=ACCENT)
    fg.save("fig02_datenfluss.svg", "Datenfluss der Ableitungsinformation")


# =============================================================================
# Abb. 3/4: Sequenzdiagramme
# =============================================================================

class Seq:
    def __init__(self, fig, parts, top=15, bottom=None):
        self.fg = fig
        self.x = {}
        for name, x, label, fill, stroke in parts:
            self.x[name] = x
            fig.box(x - 85, top, 170, 40, label, fill=fill, stroke=stroke, bold_first=True, size=13)
            fig.polyline([(x, top + 40), (x, bottom)], GRAY, 1.0, dash="5,4")

    def activation(self, name, y0, y1, fill):
        x = self.x[name]
        self.fg.rect(x - 6, y0, 12, y1 - y0, fill, INK, 1.0, 0)

    def msg(self, a, b, y, labels, ret=False, color=INK, label_color=None):
        xa, xb = self.x[a], self.x[b]
        off = 7 if a != "M" else 0
        offb = 7 if b != "M" else 0
        sgn = 1 if xb > xa else -1
        x0 = xa + sgn * (6 if off else 0)
        x1 = xb - sgn * (6 if offb else 0)
        self.fg.line([(x0, y), (x1, y)], color, 1.4, dash="6,4" if ret else None)
        labels = labels if isinstance(labels, list) else [labels]
        lx = min(xa, xb) + 14
        for k, lab in enumerate(labels):
            col = label_color[k] if isinstance(label_color, list) else (label_color or INK)
            self.fg.text(lx, y - 8 - (len(labels) - 1 - k) * 16.5, lab, size=13, fill=col, halo=True)

    def note(self, x0, x1, y, h, text, fill=PANEL):
        self.fg.box(x0, y, x1 - x0, h, text, fill=fill, stroke=GRAY, size=13, sw=1.0)


def seq_parts():
    return [("M", 100, ["Master"], PANEL, GRAY),
            ("A", 420, ["FMU A", ], ACCENT_FILL, ACCENT),
            ("B", 700, ["FMU B"], ORANGE_FILL, ORANGE)]


def fig_seq_jacobi():
    fg = Fig(800, 500)
    s = Seq(fg, seq_parts(), bottom=490)
    s.note(30, 770, 72, 28, "Gauss-Jacobi: Kommunikationsschritt t_n → t_{n+1} = t_n + h")
    s.msg("M", "A", 150, ["`fmi2SetReal`(u_A(t_n))",
                          "`fmi2SetRealInputDerivatives`(du_A/dt(t_n))"],
          label_color=[INK, ACCENT])
    s.msg("M", "B", 210, ["`fmi2SetReal`(c_0 + c_1·y_A(t_n))",
                          "`fmi2SetRealInputDerivatives`(c_1·dy_A/dt(t_n))"],
          label_color=[INK, ACCENT])
    s.activation("A", 245, 300, ACCENT_FILL)
    s.activation("B", 275, 330, ORANGE_FILL)
    s.msg("M", "A", 245, "`fmi2DoStep`(t_n, h)")
    s.msg("M", "B", 275, "`fmi2DoStep`(t_n, h)")
    s.msg("A", "M", 375, ["`fmi2GetReal`, `fmi2GetRealOutputDerivatives`",
                          "y_A(t_{n+1}), dy_A/dt(t_{n+1})"], ret=True, label_color=[INK, ACCENT])
    s.msg("B", "M", 430, ["`fmi2GetReal`, `fmi2GetRealOutputDerivatives`",
                          "y_B(t_{n+1}), dy_B/dt(t_{n+1})"], ret=True, label_color=[INK, ACCENT])
    s.note(20, 420, 448, 30, "Werte und Ableitungen bei t_{n+1} → `realytNext`, `realDerivytNext`",
           fill=WHITE)
    fg.save("fig03_sequenz_gauss_jacobi.svg", "Sequenzdiagramm Gauss-Jacobi mit Ableitungen")


def fig_seq_seidel():
    fg = Fig(800, 540)
    s = Seq(fg, seq_parts(), bottom=530)
    s.note(30, 770, 72, 28, "Gauss-Seidel (A vor B): Kommunikationsschritt t_n → t_{n+1} = t_n + h")
    s.msg("M", "A", 150, ["`fmi2SetReal`(u_A(t_n))",
                          "`fmi2SetRealInputDerivatives`(du_A/dt(t_n))"],
          label_color=[INK, ACCENT])
    s.activation("A", 185, 235, ACCENT_FILL)
    s.msg("M", "A", 185, "`fmi2DoStep`(t_n, h)")
    s.msg("A", "M", 270, ["`fmi2GetReal`, `fmi2GetRealOutputDerivatives`",
                          "y_A(t_{n+1}), dy_A/dt(t_{n+1})"], ret=True, label_color=[INK, ACCENT])
    s.note(20, 360, 288, 30, "Sekante  s = (y_A(t_{n+1}) − y_A(t_n)) / h", fill=ACCENT_FILL)
    s.msg("M", "B", 375, ["`fmi2SetReal`(c_0 + c_1·y_A(t_n))",
                          "`fmi2SetRealInputDerivatives`(c_1·s)"],
          label_color=[INK, ACCENT])
    s.activation("B", 415, 465, ORANGE_FILL)
    s.msg("M", "B", 415, "`fmi2DoStep`(t_n, h)")
    s.msg("B", "M", 505, ["`fmi2GetReal`, `fmi2GetRealOutputDerivatives`",
                          "y_B(t_{n+1}), dy_B/dt(t_{n+1})"], ret=True, label_color=[INK, ACCENT])
    fg.save("fig04_sequenz_gauss_seidel.svg", "Sequenzdiagramm Gauss-Seidel mit Sekante")


# =============================================================================
# Abb. 5: Zeitbezug bei Gauss-Seidel
# =============================================================================

def fig_gs_timing():
    fg = Fig(660, 330)

    def y(t):
        return 0.2 + 0.9 * t - 0.35 * t * t

    def dy(t):
        return 0.9 - 0.7 * t

    ax = Axes(fg, 60, 40, 470, 280, (-0.35, 1.15), (0, 1.2))
    fg.rect(ax.X(0), ax.y0, ax.X(1) - ax.X(0), ax.y1 - ax.y0, PANEL, "none", 0, 0)
    fg.text((ax.X(0) + ax.X(1)) / 2, ax.y0 - 8, "Kommunikationsschritt h", size=12, anchor="middle", fill=GRAY)
    for t in (0, 1):
        fg.polyline([(ax.X(t), ax.y0), (ax.X(t), ax.y1)], GRAY, 1.0, dash="4,3")
    fg.text(ax.X(0), ax.y1 + 18, "t_n", size=13, anchor="middle", italic=True)
    fg.text(ax.X(1), ax.y1 + 18, "t_{n+1}", size=13, anchor="middle", italic=True)

    # Kurve
    fg.polyline([ax.P(-0.2 + k * 0.01, y(-0.2 + k * 0.01)) for k in range(0, 129)], INK, 1.5)
    fg.text(ax.X(-0.33), ax.Y(0.62), ["wahrer", "Verlauf y_A(t)"], size=12)
    fg.polyline([(ax.X(-0.25), ax.Y(0.5)), ax.P(-0.12, y(-0.12))], INK, 0.8)

    y0, y1 = y(0), y(1)
    # GJ: Extrapolation ab t_n
    fg.polyline([ax.P(0, y0), ax.P(1, y0 + dy(0))], GRAY, 1.8, dash="7,4")
    # GS-2: Extrapolation vom falschen Ende
    fg.polyline([ax.P(0, y1), ax.P(1, y1 + dy(1))], RED, 2.0, dash="2,3")
    # GS-1: Sekante
    fg.polyline([ax.P(0, y0), ax.P(1, y1)], ACCENT, 2.6)

    fg.circle(*ax.P(0, y0), r=3.6)
    fg.circle(*ax.P(1, y1), r=3.6)
    fg.circle(*ax.P(0, y1), r=3.6, fill=WHITE, stroke=RED, sw=1.6)
    fg.text(ax.X(0) - 8, ax.Y(y0) + 4, "y_A(t_n)", size=12, anchor="end")
    fg.text(ax.X(0) - 8, ax.Y(y1) + 4, "y_A(t_{n+1})", size=12, anchor="end", fill=RED)
    fg.text(ax.X(1) + 8, ax.Y(y1) + 18, "y_A(t_{n+1})", size=12)

    lx = 490
    fg.text(lx, ax.Y(y0 + dy(0)) + 4, ["Gauss-Jacobi:", "y(t_n) + dy/dt(t_n)·(t − t_n)"], size=12, fill=GRAY)
    fg.text(lx, ax.Y(y1 + dy(1)) + 10, ["GS-2 (nicht empfohlen):", "y(t_{n+1}) + dy/dt(t_{n+1})·(t − t_n)"],
            size=12, fill=RED)
    fg.text(lx, ax.Y(y1) + 22, ["GS-1 (empfohlen):", "Sekante y(t_n) → y(t_{n+1})"], size=12, fill=ACCENT,
            weight="bold")
    ax.arrows("t", "y_A, u_B")
    fg.save("fig05_zeitbezug_gauss_seidel.svg", "Zeitbezug der Eingangsableitung bei Gauss-Seidel")


# =============================================================================
# Abb. 6: Ablauf im Master
# =============================================================================

def fig_master_flow():
    fg = Fig(800, 830)
    cx = 400
    fg.box(cx - 120, 15, 240, 38, "Start: Schritt t_n → t_{n+1}", fill=PANEL, stroke=GRAY, rx=19, size=13)
    fg.line([(cx, 53), (cx, 72)])
    fg.diamond(cx, 110, 260, 76, ["Verbindung mit", "m_{conn} ≥ 1 ?"])
    # nein -> ZOH
    fg.line([(cx - 130, 110), (105, 110), (105, 268)])
    fg.text(cx - 140, 102, "nein", size=12, anchor="end", fill=GRAY)
    fg.box(20, 270, 170, 60, ["nur `fmi2SetReal`", "(Zero-Order-Hold)"], size=13)
    # ja -> Algorithmus
    fg.line([(cx, 148), (cx, 172)])
    fg.text(cx + 8, 165, "ja", size=12, fill=GRAY)
    fg.diamond(cx, 205, 230, 64, "Algorithmus ?")
    fg.line([(cx, 237), (cx, 250), (310, 250), (310, 268)])
    fg.line([(cx, 250), (540, 250), (540, 268)])
    fg.box(210, 270, 200, 76, ["Gauss-Jacobi oder", "Erzeuger noch nicht gerechnet:",
                               "u = c_0 + c_1·y(t_n)", "du/dt = c_1·dy/dt(t_n)"],
           size=12, line_colors=[INK, INK, INK, ACCENT])
    fg.box(425, 270, 235, 76, ["Gauss-Seidel,", "Erzeuger schon gerechnet:",
                               "u = c_0 + c_1·y(t_n)", "du/dt = c_1·(y(t_{n+1}) − y(t_n))/h"],
           size=12, line_colors=[INK, INK, INK, ACCENT], fill=ACCENT_FILL, stroke=ACCENT)
    fg.line([(310, 346), (310, 378)])
    fg.line([(540, 346), (540, 378)])
    fg.box(230, 380, 450, 38, "`fmi2SetReal` + `fmi2SetRealInputDerivatives`", size=13)
    fg.line([(cx, 418), (cx, 448)])
    fg.line([(105, 330), (105, 469), (cx - 110, 469)])
    fg.box(cx - 110, 450, 220, 38, "`fmi2DoStep`(t_n, h)", size=13, fill=ORANGE_FILL, stroke=ORANGE)
    fg.line([(cx, 488), (cx, 513)])
    fg.box(cx - 210, 515, 420, 54, ["`fmi2GetReal` + `fmi2GetRealOutputDerivatives`",
                                    "→ `realytNext`, `realDerivytNext`"], size=13)
    fg.line([(cx, 569), (cx, 592)])
    fg.diamond(cx, 625, 230, 64, "Iteration nötig ?")
    # ja -> Rollback
    fg.line([(cx + 115, 625), (598, 625)])
    fg.text(cx + 124, 617, "ja", size=12, fill=GRAY)
    fg.box(600, 590, 185, 70, ["Rollback:", "`fmi2SetFMUstate`", "Eingänge und Ableitungen", "neu setzen"],
           size=12, fill=PANEL, stroke=GRAY)
    fg.line([(692, 590), (692, 205), (cx + 117, 205)])
    # nein -> akzeptieren
    fg.line([(cx, 657), (cx, 688)])
    fg.text(cx + 8, 678, "nein", size=12, fill=GRAY)
    fg.box(cx - 175, 690, 350, 54, ["Schritt akzeptieren:", "`realyt` ← `realytNext`, `realDerivyt` ← `realDerivytNext`"],
           size=12, bold_first=True)
    fg.line([(cx, 744), (cx, 770)])
    fg.box(cx - 90, 772, 180, 38, "Ende Schritt", fill=PANEL, stroke=GRAY, rx=19, size=13)
    fg.save("fig06_ablauf_master.svg", "Ablauf eines Kommunikationsschritts im Master")


# =============================================================================
# Abb. 7: FMU als Erzeuger (Zustaende)
# =============================================================================

def fig_producer_states():
    fg = Fig(800, 330)
    # Startpunkt
    fg.circle(30, 165, 8)
    fg.line([(38, 165), (258, 165)])
    fg.text(48, 155, "`fmi2ExitInitializationMode`", size=12)
    # Zustand am Kommunikationspunkt
    fg.box(260, 125, 250, 80, ["Am Kommunikationspunkt t_n", "Zustand x(t_n), Ausgänge y(t_n),",
                               "Ableitungsinfo dy/dt(t_n)"],
           fill=ACCENT_FILL, stroke=ACCENT, bold_first=True, size=13, rx=14)
    # Rechnet
    fg.box(630, 130, 150, 70, ["Rechnet", "integriert", "t_n → t_n + h"], bold_first=True, size=13, rx=14)
    fg.path("M510,145 C560,120 590,125 628,145", end=True)
    fg.text(570, 116, "`fmi2DoStep`(t_n, h)", size=12, anchor="middle")
    fg.path("M630,185 C590,210 560,210 512,187", end=True)
    fg.text(570, 228, ["t_n ← t_n + h,", "Ableitungsinfo aktualisiert"], size=12, anchor="middle")
    # Selbstschleifen
    fg.path("M330,125 C320,70 450,70 440,123", end=True, color=ACCENT)
    fg.text(385, 50, ["`fmi2GetReal`(y), `fmi2GetRealOutputDerivatives`(y, k)",
                      "Ableitung aus dem Zustand bei t_n"], size=12, anchor="middle", fill=ACCENT)
    fg.path("M330,205 C320,262 450,262 440,207", end=True, color=GRAY)
    fg.text(385, 285, ["`fmi2SetFMUstate`(…)", "Zustand und Ableitungsinfo wiederhergestellt"],
            size=12, anchor="middle", fill=GRAY)
    fg.save("fig07_fmu_erzeuger.svg", "FMU als Erzeuger von Ausgangsableitungen")


# =============================================================================
# Abb. 8: FMU als Verbraucher (Varianten A und B)
# =============================================================================

def fig_consumer_variants():
    fg = Fig(800, 430)
    for px, title in ((15, "Variante A: Master liefert Ableitung"), (410, "Variante B: FMU-interne Historie")):
        fg.rect(px, 10, 375, 410, PANEL, GRAY, 1.0, 6)
        fg.text(px + 14, 34, title, size=14, weight="bold")
    # A
    ax = 50
    fg.box(ax, 55, 305, 34, "`fmi2SetReal`(u(t_n))", size=13)
    fg.box(ax, 99, 305, 34, "`fmi2SetRealInputDerivatives`(du/dt)", size=13, fill=ACCENT_FILL, stroke=ACCENT)
    fg.line([(ax + 152, 133), (ax + 152, 163)])
    fg.box(ax, 165, 305, 50, ["Eingangspolynom", "u(t) = u(t_n) + du/dt·(t − t_n)"], size=13)
    fg.line([(ax + 152, 215), (ax + 152, 245)])
    fg.box(ax, 247, 305, 34, "integrieren t_n → t_{n+1}", size=13, fill=ORANGE_FILL, stroke=ORANGE)
    fg.text(ax + 152, 330, ["Zeitbezug eindeutig:", "Werte und Ableitungen gelten",
                            "am Schrittanfang t_n (FMI 2.0.5, 4.2.1)"],
            size=12, anchor="middle", fill=ACCENT)
    # B
    bx = 445
    fg.box(bx, 55, 305, 34, "`fmi2SetReal`(u)", size=13)
    fg.line([(bx + 152, 89), (bx + 152, 113)])
    fg.box(bx, 115, 305, 34, "Historie (t_{prev}, u_{prev})", size=13)
    fg.line([(bx + 152, 149), (bx + 152, 173)])
    fg.box(bx, 175, 305, 50, ["du/dt ≈ (u − u_{prev}) / (t − t_{prev})", "oder Rampe u_{prev} → u"], size=13)
    fg.line([(bx + 152, 225), (bx + 152, 249)])
    fg.box(bx, 251, 305, 34, "integrieren t_n → t_{n+1}", size=13, fill=ORANGE_FILL, stroke=ORANGE)
    fg.line([(bx + 152, 285), (bx + 152, 309)])
    fg.box(bx, 311, 305, 34, "Historie aktualisieren (im FMUstate!)", size=13)
    fg.line([(bx + 305, 328), (bx + 325, 328), (bx + 325, 132), (bx + 307, 132)], GRAY, 1.2, dash="5,3")
    fg.text(bx + 152, 375, ["Zeitbezug muss geraten werden:", "Schrittanfang oder -ende?"],
            size=12, anchor="middle", fill=RED)
    fg.save("fig08_fmu_verbraucher.svg", "FMU als Verbraucher von Eingangsableitungen")


# =============================================================================
# Abb. 9: Room/Radiator-Kopplung
# =============================================================================

def fig_room_radiator():
    fg = Fig(800, 260)
    fg.box(15, 40, 310, 140, ["Radiator", "P-Regler mit Stufen-Thermostat",
                              "Q = clamp(K_p·(T_{set} − T_{est}), 0, Q_{max})",
                              "K_p = 2000 W/K,  Q_{max} = 10 kW",
                              "T_{set} = 22 °C / 16 °C (je halber Tag)"],
           fill=ACCENT_FILL, stroke=ACCENT, bold_first=True, size=13)
    fg.box(475, 40, 310, 140, ["Room", "Energiebilanz des Raums",
                               "C·dT/dt = Q − k·(T − T_{amb})",
                               "C = 50 000 J/K,  k = 250 W/K",
                               "T_0 = 2 °C"],
           fill=ORANGE_FILL, stroke=ORANGE, bold_first=True, size=13)
    fg.line([(325, 75), (473, 75)], INK, 1.6)
    fg.text(400, 66, "`heatingLoad`  Q [W]", size=12, anchor="middle")
    fg.line([(475, 145), (327, 145)], INK, 1.6)
    fg.text(400, 136, "`roomTemperature`  T [K]", size=12, anchor="middle")
    fg.line([(630, 235), (630, 182)], GRAY, 1.4)
    fg.text(620, 232, "T_{amb} = 2 °C (konstant, nicht verbunden)", size=12, fill=GRAY, anchor="end")
    fg.text(15, 205, "T_{est}: aus der Eingangshistorie extrapolierte Raumtemperatur", size=12, fill=GRAY)
    fg.text(15, 22, "beide FMUs: `canInterpolateInputs=\"true\"`, `maxOutputDerivativeOrder=\"1\"`",
            size=12, fill=GRAY)
    fg.save("fig09_room_radiator.svg", "Kopplung der Test-FMUs Room und Radiator")


# =============================================================================
# Abb. 10: Interpretation der Eingaenge in Room und Radiator
# =============================================================================

def fig_fmu_interpretation():
    fg = Fig(800, 280)
    # (a) Room
    px = 12
    fg.text(px, 18, "(a) Room: Eingang Q", size=14, weight="bold")
    fg.text(px, 36, "gesetzter Wert gilt als Q(t_{n+1}), Rampe ab gemerktem Q(t_n)", size=12, fill=GRAY)
    ax = Axes(fg, px + 22, 62, px + 365, 225, (-0.25, 2.35), (0, 1.0))
    Q = {0: 0.3, 1: 0.5, 2: 0.82}
    fg.rect(ax.X(1), ax.y0, ax.X(2) - ax.X(1), ax.y1 - ax.y0, PANEL, "none", 0, 0)
    fg.polyline([ax.P(0, Q[0]), ax.P(1, Q[1])], ACCENT, 1.4, dash="4,3")
    fg.polyline([ax.P(1, Q[2]), ax.P(2, Q[2])], GRAY, 1.6, dash="7,4")
    fg.polyline([ax.P(1, Q[1]), ax.P(2, Q[2])], ACCENT, 2.6)
    for k in range(3):
        fg.circle(*ax.P(k, Q[k]), r=3.6)
    fg.text(ax.X(1) + 8, ax.Y(Q[1]) + 16, "Q(t_n), aus Vorschritt gemerkt", size=12)
    fg.text(ax.X(2) - 4, ax.Y(Q[2]) - 10, "vor `fmi2DoStep` gesetzt", size=12, anchor="end")
    fg.text(ax.X(1) - 6, ax.Y(Q[2]) + 4, "ohne Interpolation", size=12, fill=GRAY, anchor="end")
    for k, lab in enumerate(("t_{n−1}", "t_n", "t_{n+1}")):
        fg.text(ax.X(k), ax.y1 + 18, lab, size=13, anchor="middle", italic=True)
        fg.polyline([(ax.X(k), ax.y1), (ax.X(k), ax.y1 + 4)], INK, 1.0)
    ax.arrows("t", "Q")

    # (b) Radiator
    px = 412
    fg.text(px, 18, "(b) Radiator: Eingang T_{room}", size=14, weight="bold")
    fg.text(px, 36, "gesetzter Wert gilt als T(t_n), lineare Extrapolation bis t_{n+1}", size=12, fill=GRAY)
    ax = Axes(fg, px + 22, 62, px + 365, 225, (-0.25, 2.35), (0, 1.0))
    T = {0: 0.25, 1: 0.5, 2: 0.75}
    fg.rect(ax.X(1), ax.y0, ax.X(2) - ax.X(1), ax.y1 - ax.y0, PANEL, "none", 0, 0)
    fg.polyline([ax.P(1, T[1]), ax.P(2, T[1])], GRAY, 1.6, dash="7,4")
    fg.polyline([ax.P(0, T[0]), ax.P(1, T[1])], ACCENT, 1.4)
    fg.polyline([ax.P(1, T[1]), ax.P(2, T[2])], ACCENT, 2.6, dash="8,4")
    fg.circle(*ax.P(0, T[0]), r=3.6)
    fg.circle(*ax.P(1, T[1]), r=3.6)
    fg.circle(*ax.P(2, T[2]), r=4.2, fill=WHITE, stroke=ACCENT, sw=1.8)
    fg.text(ax.X(0) + 8, ax.Y(T[0]) + 16, "T(t_{n−1})", size=12)
    fg.text(ax.X(1) - 8, ax.Y(T[1]) - 8, "T(t_n), gesetzt", size=12, anchor="end")
    fg.text(ax.X(2) - 6, ax.Y(T[2]) - 12, "T_{est}(t_{n+1})", size=12, anchor="end", fill=ACCENT)
    fg.text(ax.X(2) - 6, ax.Y(T[1]) + 16, "ohne Interpolation", size=12, anchor="end", fill=GRAY)
    fg.text(ax.X(1.0) + 8, ax.y1 - 10, "Q_{n+1} = K_p·(T_{set} − T_{est})", size=12)
    for k, lab in enumerate(("t_{n−1}", "t_n", "t_{n+1}")):
        fg.text(ax.X(k), ax.y1 + 18, lab, size=13, anchor="middle", italic=True)
        fg.polyline([(ax.X(k), ax.y1), (ax.X(k), ax.y1 + 4)], INK, 1.0)
    ax.arrows("t", "T")
    fg.save("fig10_interpretation_room_radiator.svg", "Interpretation der Eingänge in Room und Radiator")


# =============================================================================
# Abb. 11: Ergebnisse der Testfaelle
# =============================================================================

def read_values(case):
    path = os.path.join(TESTDIR, case, "results", "values.csv")
    t, q, T = [], [], []
    with open(path, encoding="utf-8") as fh:
        rows = csv.reader(fh, delimiter="\t")
        header = next(rows)
        iq = [i for i, h in enumerate(header) if "heatingLoad" in h][0]
        iT = [i for i, h in enumerate(header) if "roomTemperature" in h][0]
        for r in rows:
            if not r:
                continue
            t.append(float(r[0]))
            q.append(float(r[iq]))
            T.append(float(r[iT]) - 273.15)
    return t, q, T


def fig_results():
    tmax = 900.0
    series = [
        ("Room_Radiator_Reference", "Referenz (h = 5 s)", LIGHTGRAY, 2.4, None),
        ("Room_Radiator_NoInterpolation_GaussSeidel_1iters", "ohne Interpolation (h = 30 s)", ORANGE, 1.4, "square"),
        ("Room_Radiator_GaussSeidel_1iters", "mit interner Interpolation (h = 30 s)", ACCENT, 1.4, "circle"),
    ]
    data = []
    for case, label, col, sw, mk in series:
        t, q, T = read_values(case)
        sel = [i for i, v in enumerate(t) if v <= tmax + 1e-9]
        data.append(([t[i] for i in sel], [q[i] for i in sel], [T[i] for i in sel], label, col, sw, mk))

    fg = Fig(800, 580)
    # Legende
    lx = 80
    for _, _, _, label, col, sw, mk in data:
        fg.polyline([(lx, 20), (lx + 30, 20)], col, max(sw, 1.6))
        if mk:
            marker(fg, lx + 15, 20, mk, col)
        fg.text(lx + 38, 24, label, size=12)
        lx += 52 + len(label) * 6.4

    def plot(ax, idx):
        for tt, q, T, label, col, sw, mk in data:
            vals = q if idx == 1 else T
            fg.polyline([ax.P(a, b) for a, b in zip(tt, vals)], col, sw)
            if mk:
                for a, b in zip(tt, vals):
                    marker(fg, ax.X(a), ax.Y(b), mk, col)

    xt = list(range(0, 901, 100))
    ax1 = Axes(fg, 80, 45, 780, 265, (0, tmax), (0, 10500))
    ax1.frame(xt, [0, 2000, 4000, 6000, 8000, 10000], lambda v: "", de_num)
    fg.polyline([(ax1.x0, ax1.Y(4444.4)), (ax1.x1, ax1.Y(4444.4))], INK, 0.9, dash="2,3")
    fg.text(ax1.X(8), ax1.Y(4444.4) - 7, "stationär 4444 W", size=12, halo=True)
    plot(ax1, 1)
    fg.text(22, (ax1.y0 + ax1.y1) / 2, "Heizleistung Q [W]", size=13, anchor="middle", rotate=-90)

    ax2 = Axes(fg, 80, 300, 780, 520, (0, tmax), (0, 25))
    ax2.frame(xt, [0, 5, 10, 15, 20, 25], lambda v: de_num(v), lambda v: "%d" % v)
    plot(ax2, 2)
    fg.text(22, (ax2.y0 + ax2.y1) / 2, "Raumtemperatur T [°C]", size=13, anchor="middle", rotate=-90)
    fg.text((ax2.x0 + ax2.x1) / 2, 560, "Zeit t [s]", size=13, anchor="middle")
    fg.save("fig11_ergebnisse_room_radiator.svg", "Ergebnisse der Room/Radiator-Testfälle")


def marker(fg, x, y, kind, col):
    if kind == "circle":
        fg.circle(x, y, 3.0, fill=WHITE, stroke=col, sw=1.4)
    else:
        fg.rect(x - 2.8, y - 2.8, 5.6, 5.6, WHITE, col, 1.4, 0)


if __name__ == "__main__":
    fig_hold_schemes()
    fig_dataflow()
    fig_seq_jacobi()
    fig_seq_seidel()
    fig_gs_timing()
    fig_master_flow()
    fig_producer_states()
    fig_consumer_variants()
    fig_room_radiator()
    fig_fmu_interpretation()
    fig_results()
