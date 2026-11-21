#!/usr/bin/env python3
"""Generates the comparison page (index.html) from the rendered screenshots.

Usage: make_page.py <output-dir>

Expects the screenshots in <output-dir>/shots/<variant>/<light|dark>/<section>.png
(as created by render_shots.py) and writes <output-dir>/index.html.
The page references the screenshots relatively, so the output directory can be
zipped and viewed offline.
"""
import html
import json
import struct
import sys
from pathlib import Path

if len(sys.argv) != 2:
    sys.exit(__doc__)

ROOT = Path(sys.argv[1])
SHOTS = ROOT / "shots"

VARIANTS = [
    ("original", "MasterSim", "randrew / Qt-Widgets, Qt6-patched", "Used now",
     "chip-neutral", True),
    ("julienmaille", "JulienMaille", "89 commits ahead, last 2026-09", "Most active",
     "chip-good", True),
    ("cristianadam", "cristianadam", "14 commits ahead, last 2025-12", "Closest to MasterSim",
     "chip-good", True),
    ("vicus", "VICUS", "SIM-VICUS externals/phantom, last 2026-11", "IBK in-house",
     "chip-good", True),
    ("jcelerier", "jcelerier", "30 commits ahead, last 2026-10", "Tuned for one app",
     "chip-warn", False),
    ("fusion", "Fusion", "Qt built-in, for reference", "Reference",
     "chip-neutral", False),
]

SECTIONS = [
    ("menubar_toolbar", "Menu bar and tool bar",
     "The R button is checked; X is disabled; Mode is a menu button.",
     [("julienmaille", "The checked tool button gets a highlight-tinted fill and outline. The menu arrow is a chevron."),
      ("cristianadam", "Same as MasterSim."),
      ("vicus", "Flat tool buttons; the checked one is filled with the accent color. Chevron menu arrow."),
      ("jcelerier", "The checked state of the R button is not visible, and the Mode button has no menu arrow.")]),
    ("buttons", "Buttons, check boxes, radio buttons",
     "Pressed uses setDown(true); Toggled is a checked checkable button.",
     [("julienmaille", "Pressed buttons fill with the highlight color and toggled buttons get a light tint. The partial check box is a filled square. The icon on 'With icon' is blurry at 2× scaling."),
      ("cristianadam", "Same as MasterSim."),
      ("vicus", "Fluent-like look: larger corner radius, the default button marked by an accent ring, Pressed and Toggled only slightly darker. Check boxes and radio buttons are filled with the accent color, and state changes are animated."),
      ("jcelerier", "Flat buttons. In the light palette, checked check boxes and radio buttons are nearly invisible and the Pressed label is hard to read.")]),
    ("inputs", "Line edits, combo boxes, spin boxes",
     "",
     [("julienmaille", "Chevron arrows on combo boxes and spin boxes."),
      ("cristianadam", "Same as MasterSim."),
      ("vicus", "Rounded fields with chevron arrows; spin box arrows stacked in one column."),
      ("jcelerier", "Combo boxes use the line-edit background, and the arrows are lighter.")]),
    ("sliders_progress", "Sliders, progress bars, scroll bars",
     "The second progress bar is indeterminate (busy), captured as a single frame.",
     [("julienmaille", "The slider groove fills with the highlight color up to the handle. Thin scroll bars without arrow buttons. The busy bar is animated."),
      ("cristianadam", "Thin scroll bars without arrow buttons; sliders unchanged."),
      ("vicus", "Accent-filled slider track with ring handles and stronger ticks, rounded progress bars, overlay scroll bars that widen on hover. Scroll bars outside a scroll area paint no background (the white strips in the dark screenshot); inside views this does not happen. The busy bar is animated and was captured near the start."),
      ("jcelerier", "Classic scroll bars with a lighter handle.")]),
    ("tabs_groupbox", "Tabs and group boxes",
     "'Output' is a checkable group box (checked); the Simulation tab is disabled.",
     [("julienmaille", "Group box titles sit on the frame line (Windows style). Blue highlight bar on the active tab."),
      ("cristianadam", "The same blue highlight bar on the active tab; group boxes unchanged."),
      ("vicus", "Underline tabs: no tab borders and no frame around the page, with an accent bar under the active tab. Group boxes have rounded frames."),
      ("jcelerier", "The checkable group box looks unchecked in the light palette.")]),
    ("groupboxes", "Group box variants",
     "Normal, flat, checkable (checked/unchecked), disabled and untitled group boxes, "
     "plus nested group boxes with a centered outer title. Unchecking a checkable group box disables its children.",
     # TODO: add observations per variant after rendering the screenshots
     []),
    ("itemviews", "Tree and table views",
     "Tree items have check boxes (x1 is checked and selected). The table is sorted by Output.",
     [("original", "In the dark palette, the unchecked item check boxes almost disappear."),
      ("julienmaille", "Chevron branch arrows. The sort indicator sits above the header text."),
      ("cristianadam", "Chevron branch arrows, taller rows, clearly drawn item check boxes."),
      ("vicus", "Rounded view frames. The selection is a soft accent tint with a short accent bar at the start of the row. Accent-filled item check boxes, chevron arrows."),
      ("jcelerier", "Checked item check boxes are drawn as a cross.")]),
    ("menu", "Popup menu",
     "'Show grid' and 'Auto-run' are both checked; 'Auto-run' also has an icon. The last entry is highlighted.",
     [("original", "Check mark drawn to the left of the icon."),
      ("julienmaille", "For checked items with an icon, the check mark is hidden and the icon gets a frame. More vertical spacing."),
      ("cristianadam", "For checked items with an icon, only the icon is drawn, so the checked state is not visible."),
      ("vicus", "Rounded popup with a neutral grey hover highlight instead of the accent color. Check mark and icon are both drawn."),
      ("jcelerier", "Same as MasterSim.")]),
]


# only show variants for which screenshots exist (e.g. VICUS is skipped without local checkout)
VARIANTS = [v for v in VARIANTS if (SHOTS / v[0]).is_dir()]
_present = {v[0] for v in VARIANTS}
SECTIONS = [(s, title, setup, [(v, t) for v, t in notes if v in _present])
            for s, title, setup, notes in SECTIONS]


def png_size(p: Path):
    with open(p, "rb") as f:
        head = f.read(24)
    w, h = struct.unpack(">II", head[16:24])
    return w, h


dims = {}
files = []
for v, *_ in VARIANTS:
    for t in ("light", "dark"):
        for s, *_ in SECTIONS:
            p = SHOTS / v / t / f"{s}.png"
            w, h = png_size(p)
            dims[f"{v}/{t}/{s}"] = [w // 2, h // 2]
            files.append(f"shots/{v}/{t}/{s}.png")

E = html.escape
names = {v: n for v, n, *_ in VARIANTS}

cards = []
for v, name, meta, chip, chipcls, on in VARIANTS:
    cards.append(f"""
      <label class="variant" for="v-{v}">
        <input type="checkbox" id="v-{v}" data-variant="{v}"{' checked' if on else ''}>
        <span class="variant-body">
          <span class="variant-top"><span class="variant-name">{E(name)}</span><span class="chip {chipcls}">{E(chip)}</span></span>
          <span class="variant-meta">{E(meta)}</span>
        </span>
      </label>""")

sections = []
for s, title, setup, notes in SECTIONS:
    figs = []
    for v, name, *_ in VARIANTS:
        w, h = dims[f"{v}/light/{s}"]
        figs.append(f"""
          <figure class="shot" data-variant="{v}">
            <figcaption>{E(name)}</figcaption>
            <div class="frame"><img src="shots/{v}/light/{s}.png" data-section="{s}" data-v="{v}" width="{w}" height="{h}" alt="{E(title)} rendered with {E(name)}" loading="lazy"></div>
          </figure>""")
    note_items = "".join(
        f'<li data-variant="{v}"><span class="tag tag-{v}">{E(names[v])}</span> {E(text)}</li>'
        for v, text in notes)
    sections.append(f"""
    <section class="sec" id="{s}">
      <div class="sec-head">
        <h2>{E(title)}</h2>
        {f'<p class="setup">{E(setup)}</p>' if setup else ''}
      </div>
      <ul class="notes">{note_items}</ul>
      <div class="strip" tabindex="0" aria-label="{E(title)} screenshots">{''.join(figs)}
      </div>
    </section>""")

page = f"""<!doctype html>
<html lang="en">
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Phantom Fork Comparison</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;500&family=IBM+Plex+Sans:wght@400;500;600&display=swap">
<style>
/* Layout: a header with a short verdict, a sticky control bar, then one row of
   side-by-side screenshots per widget group, with the observed differences above each row. */
:root {{
  --bg: #f3f4f6;
  --surface: #ffffff;
  --fg: #1c2230;
  --muted: #5b6474;
  --line: #d9dde4;
  --accent: #2f6f8f;
  --accent-soft: #e2eef4;
  --good: #2e7d4f;
  --good-soft: #e3f2e8;
  --warn: #9a6200;
  --warn-soft: #fbf0d9;
  --neutral-soft: #eceef2;
  --shot-light: #e8e9ec;
  --shot-dark: #232528;
  --font-body: "IBM Plex Sans", system-ui, -apple-system, "Segoe UI", sans-serif;
  --font-mono: "IBM Plex Mono", ui-monospace, "SFMono-Regular", Menlo, monospace;
}}
@media (prefers-color-scheme: dark) {{
  :root:not([data-theme="light"]) {{
    --bg: #15181d; --surface: #1d2128; --fg: #e4e7ec; --muted: #9aa3b2; --line: #313743;
    --accent: #7fb6d3; --accent-soft: #1f3340; --good: #7ccf9c; --good-soft: #1c3326;
    --warn: #e7b65c; --warn-soft: #3a2e17; --neutral-soft: #262b33;
    --shot-light: #d9dbdf; --shot-dark: #0f1113;
    color-scheme: dark;
  }}
}}
:root[data-theme="dark"] {{
  --bg: #15181d; --surface: #1d2128; --fg: #e4e7ec; --muted: #9aa3b2; --line: #313743;
  --accent: #7fb6d3; --accent-soft: #1f3340; --good: #7ccf9c; --good-soft: #1c3326;
  --warn: #e7b65c; --warn-soft: #3a2e17; --neutral-soft: #262b33;
  --shot-light: #d9dbdf; --shot-dark: #0f1113;
  color-scheme: dark;
}}
* {{ box-sizing: border-box; }}
body {{
  margin: 0;
  background: var(--bg); color: var(--fg);
  font-family: var(--font-body); font-size: 15px; line-height: 1.5;
}}
body::before {{
  content: ""; position: fixed; inset: 0 0 auto; height: env(safe-area-inset-top, 0px);
  z-index: 10; background: var(--surface);
}}
.wrap {{
  margin: 0 auto;
  padding-left: max(16px, env(safe-area-inset-left, 0px));
  padding-right: max(16px, env(safe-area-inset-right, 0px));
}}
header.top {{ padding-block: 32px 20px; }}
h1 {{ font-size: 28px; font-weight: 600; margin: 0 0 8px; text-wrap: balance; letter-spacing: -0.01em; }}
.lede {{ margin: 0; max-width: 72ch; color: var(--muted); }}
.lede code, .setup code {{ font-family: var(--font-mono); font-size: 0.9em; }}

.controls {{
  position: sticky; top: env(safe-area-inset-top, 0px); z-index: 5;
  background: var(--surface); border-bottom: 1px solid var(--line);
  padding-block: 12px;
}}
.controls .wrap {{ display: flex; flex-wrap: wrap; gap: 12px 24px; align-items: center; }}
.ctl-label {{
  font-size: 12px; font-weight: 600; text-transform: uppercase; letter-spacing: 0.06em; color: var(--muted);
}}
.group {{ display: flex; gap: 10px; align-items: center; flex-wrap: wrap; min-width: 0; }}
.seg {{ display: inline-flex; border: 1px solid var(--line); border-radius: 6px; overflow: hidden; }}
.seg button {{
  font: inherit; font-size: 14px; padding: 6px 14px; border: 0; background: transparent; color: var(--fg); cursor: pointer;
}}
.seg button + button {{ border-left: 1px solid var(--line); }}
.seg button[aria-pressed="true"] {{ background: var(--accent); color: var(--surface); }}
.seg button:focus-visible, .variant:focus-within, .strip:focus-visible {{ outline: 2px solid var(--accent); outline-offset: 2px; }}

.variants {{ display: flex; flex-wrap: wrap; gap: 8px; }}
.variant {{
  display: flex; gap: 8px; align-items: flex-start; cursor: pointer;
  border: 1px solid var(--line); border-radius: 6px; padding: 6px 10px; background: var(--bg);
}}
.variant input {{ margin: 3px 0 0; accent-color: var(--accent); }}
.variant-body {{ display: flex; flex-direction: column; }}
.variant-top {{ display: flex; gap: 8px; align-items: center; }}
.variant-name {{ font-weight: 600; font-size: 14px; }}
.variant-meta {{ font-size: 12px; color: var(--muted); font-variant-numeric: tabular-nums; }}
.chip {{ font-size: 11px; font-weight: 500; padding: 1px 7px; border-radius: 999px; white-space: nowrap; }}
.chip-good {{ background: var(--good-soft); color: var(--good); }}
.chip-warn {{ background: var(--warn-soft); color: var(--warn); }}
.chip-neutral {{ background: var(--neutral-soft); color: var(--muted); }}

main {{ padding-block: 8px 48px; }}
.sec {{ padding-block: 28px; border-bottom: 1px solid var(--line); }}
.sec-head {{ display: flex; flex-wrap: wrap; gap: 4px 16px; align-items: baseline; }}
h2 {{ font-size: 19px; font-weight: 600; margin: 0; }}
.setup {{ margin: 0; color: var(--muted); font-size: 14px; }}
.notes {{ list-style: none; padding: 0; margin: 12px 0 16px; display: grid; gap: 6px; max-width: 110ch; }}
.notes li {{ font-size: 14px; }}
.tag {{
  display: inline-block; font-family: var(--font-mono); font-size: 12px; font-weight: 500;
  padding: 0 6px; border-radius: 4px; margin-right: 4px; background: var(--neutral-soft); color: var(--fg);
}}
.tag-julienmaille, .tag-cristianadam {{ background: var(--accent-soft); color: var(--accent); }}
.tag-jcelerier {{ background: var(--warn-soft); color: var(--warn); }}

.strip {{
  display: grid; gap: 16px; align-items: start; padding-bottom: 8px;
  grid-template-columns: repeat(var(--cols, 3), minmax(0, 1fr));
}}
.shot {{ margin: 0; min-width: 0; }}
.shot figcaption {{
  font-family: var(--font-mono); font-size: 12px; font-weight: 500; color: var(--muted); margin-bottom: 6px;
}}
.frame {{
  border: 1px solid var(--line); border-radius: 4px; overflow: hidden; line-height: 0;
  width: fit-content; max-width: 100%;
  background: var(--shot-light);
}}
body.shots-dark .frame {{ background: var(--shot-dark); }}
.frame img {{ display: block; max-width: 100%; height: auto; }}
[data-hidden-variant] {{ display: none !important; }}
.empty {{ color: var(--muted); font-size: 14px; }}

footer {{ padding-block: 24px 40px; color: var(--muted); font-size: 13px; }}
footer p {{ margin: 0 0 6px; max-width: 90ch; }}
@media (max-width: 1000px) {{
  .strip {{ grid-template-columns: repeat(2, minmax(0, 1fr)); }}
}}
@media (max-width: 640px) {{
  .strip {{ grid-template-columns: minmax(0, 1fr); }}
  h1 {{ font-size: 23px; }}
  .controls {{ position: static; }}
}}
</style>

<header class="top">
  <div class="wrap">
    <h1>Phantom Fork Comparison</h1>
    <p class="lede">The same widget gallery drawn by the Phantom style currently used in MasterSim (our Qt 6 port of the original, in
      <code>externals/phantom-style</code>), three maintained GitHub forks, the in-house SIM-VICUS variant, and Qt's Fusion style for reference.
      Rendered with Qt 6.11 at 2× scaling, Noto Sans 9 pt, and identical light and dark palettes, so every
      difference you see comes from the style itself.</p>
  </div>
</header>

<div class="controls">
  <div class="wrap">
    <div class="group">
      <span class="ctl-label" id="pal-label">Palette</span>
      <div class="seg" role="group" aria-labelledby="pal-label">
        <button type="button" id="pal-light" data-pal="light" aria-pressed="true">Light</button>
        <button type="button" id="pal-dark" data-pal="dark" aria-pressed="false">Dark</button>
      </div>
    </div>
    <div class="group">
      <span class="ctl-label">Styles</span>
      <div class="variants">{''.join(cards)}
      </div>
    </div>
  </div>
</div>

<main>
  <div class="wrap">{''.join(sections)}
  </div>
</main>

<footer>
  <div class="wrap">
    <p>Sources: randrew/phantomstyle and its GitHub forks, as of 2026-11-20. JulienMaille's fork was built with one line removed
      (<code>PM_ButtonIconLabelMargin</code>, which only exists in the author's patched Qt); all other variants compiled unchanged. VICUS was taken from <code>SIM-VICUS/externals/phantom</code> as of 2026-11-20, without the app-level style sheets VICUS adds to its item views.</p>
    <p>Hover and focus states are not shown: the screenshots come from an offscreen render with no mouse.</p>
  </div>
</footer>

<script>
(function () {{
  var DIMS = {json.dumps(dims, separators=(',', ':'))};
  var store = {{
    get: function (k) {{ try {{ return localStorage.getItem(k); }} catch (e) {{ return null; }} }},
    set: function (k, v) {{ try {{ localStorage.setItem(k, v); }} catch (e) {{}} }}
  }};

  function setPalette(pal) {{
    document.querySelectorAll('.seg button').forEach(function (b) {{
      b.setAttribute('aria-pressed', String(b.dataset.pal === pal));
    }});
    document.body.classList.toggle('shots-dark', pal === 'dark');
    document.querySelectorAll('img[data-section]').forEach(function (img) {{
      var key = img.dataset.v + '/' + pal + '/' + img.dataset.section;
      var d = DIMS[key];
      img.src = 'shots/' + key + '.png';
      if (d) {{ img.width = d[0]; img.height = d[1]; }}
    }});
    store.set('phantom-cmp-pal', pal);
  }}

  function applyVariants() {{
    var on = {{}};
    document.querySelectorAll('.variant input').forEach(function (cb) {{ on[cb.dataset.variant] = cb.checked; }});
    document.querySelectorAll('.shot[data-variant], .notes li[data-variant]').forEach(function (el) {{
      if (on[el.dataset.variant]) el.removeAttribute('data-hidden-variant');
      else el.setAttribute('data-hidden-variant', '');
    }});
    var n = Object.keys(on).filter(function (k) {{ return on[k]; }}).length;
    document.documentElement.style.setProperty('--cols', String(Math.max(n, 1)));
    store.set('phantom-cmp-variants', JSON.stringify(on));
  }}

  document.querySelectorAll('.seg button').forEach(function (b) {{
    b.addEventListener('click', function () {{ setPalette(b.dataset.pal); }});
  }});
  document.querySelectorAll('.variant input').forEach(function (cb) {{
    cb.addEventListener('change', applyVariants);
  }});

  try {{
    var saved = JSON.parse(store.get('phantom-cmp-variants') || 'null');
    if (saved) document.querySelectorAll('.variant input').forEach(function (cb) {{
      if (cb.dataset.variant in saved) cb.checked = !!saved[cb.dataset.variant];
    }});
  }} catch (e) {{}}
  applyVariants();

  var pal = store.get('phantom-cmp-pal');
  if (pal !== 'light' && pal !== 'dark') {{
    pal = (window.matchMedia && matchMedia('(prefers-color-scheme: dark)').matches) ? 'dark' : 'light';
  }}
  setPalette(pal);
}})();
</script>
"""

(ROOT / "index.html").write_text(page + "</html>\n", encoding="utf-8")
print(f"Wrote {ROOT / 'index.html'} ({len(files)} images)")
