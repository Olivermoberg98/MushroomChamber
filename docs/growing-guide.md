# The Mushroom Growing Guide — Everything Except the Electronics

Substrate, spawn, sterilisation, and sourcing for a 65 L SAMLA chamber in Gothenburg.

> **Scope.** This covers the *biology and consumables* side: what to put in your bags, what
> to inoculate it with, how to keep it from turning green, and where to buy it in Gothenburg.
> Nothing here is about the ESP32, sensors, or actuators — see [bom.csv](bom.csv) and
> [CLAUDE.md](../CLAUDE.md) for that half.
>
> **Prices checked August 2026.** Treat them as approximate; Swedish feed and pellet prices
> move with the season (pellets are cheapest in spring, dearest in October).

---

## 0. Read this first — two findings that should change your order

Before the shopping lists, two things fell out of cross-checking your firmware against your BOM.
Both affect what you buy, so they go at the top.

### 0.1 Your firmware is set to the hardest species for a first grow

`main.cpp` calls `getMushroomConfig(SHIITAKE)`. Shiitake is a genuinely bad first mushroom:

| | Shiitake | Blue/grey oyster |
|---|---|---|
| Incubation | **8–16 weeks**, plus a "browning" phase | **2–3 weeks** |
| Contamination tolerance | Low — long incubation = long exposure | **Highest of any gourmet species** |
| Substrate | Supplemented hardwood, **must be sterilised** | Straw pellets, **pasteurisation is enough** |
| Needs a pressure cooker? | Yes | No |
| Fruiting temp (your config) | 13 °C | 18 °C |
| Typical first-try success | Maybe 40 % | ~90 % |

Oyster is the species that forgives the twenty small mistakes everyone makes on grow #1. It
colonises fast enough to outrun contaminants, it fruits on cheap straw pellets you can
pasteurise in a bucket, and it fruits at a temperature your chamber can actually hold.

**Recommendation:** change the one line to `getMushroomConfig(OYSTER)` for your first run.
Come back to shiitake on run #3 once your sterile technique is proven. Everything in this guide
is written oyster-first, with shiitake notes where they differ.

> Two side notes on the config while you are in there: `ENOKI` exists in the `MushroomType` enum
> but has no `case` in `getMushroomConfig()`, so selecting it silently returns "Generic Mushroom".
> And the controller only ever reads `targetHumidity` from the phase config — the temperature
> targets in those tables are currently documentation, not control. Which leads directly to:

### 0.2 You have no way to *cool* the chamber — and that decides your season

Your BOM has a humidifier, fans, and LEDs. There is no cooling element. The chamber can only
ever sit at **ambient room temperature or slightly above** (the humidifier and electronics add
a little heat).

Now look at what the species actually want to fruit:

| Species | Fruiting target (your config) | Realistic in a heated Gothenburg flat (20–22 °C)? |
|---|---|---|
| Oyster | 18 °C ± 3 | **Yes**, just barely — top of tolerance |
| Lion's Mane | 17.5 °C ± 2.5 | **Yes**, marginal |
| King Oyster | 16.5 °C ± 1.5 | No |
| Shimeji | 15.5 °C ± 2.5 | No |
| Maitake | 15 °C ± 3 | No |
| Shiitake | 13 °C ± 3 | No |

Three practical options, cheapest first:

1. **Grow with the Swedish seasons — free.** An unheated room, garage, cellar, or glassed-in
   balcony in Gothenburg sits at 8–16 °C from roughly October to April. That is *exactly*
   shiitake and king oyster fruiting range, for zero kronor. This is the single best cooling
   "hack" available to you and it costs nothing. Incubation still needs 24–25 °C, so colonise
   the bags indoors in a warm cupboard, then move them out to the cold chamber to fruit.
   Cold-shocking is a pinning trigger for shiitake and oyster anyway — you get the temperature
   drop for free.
2. **Pick warm-fruiting species year-round.** Blue oyster (*P. ostreatus*), pink oyster
   (*P. djamor*, wants 20–28 °C and is the one species that likes a Swedish summer flat), and
   Lion's Mane all fruit acceptably at room temperature.
3. **Buy cooling — 1500–3000 kr.** A used wine fridge (*vinkyl*) on Blocket, with the chamber
   inside or the fridge itself converted, is the standard hobbyist answer. Peltier modules are
   popular in builds like yours and are almost always a disappointment: low capacity, heavy
   condensation, poor efficiency. Skip Peltier.

**Recommendation:** plan grow #1 as oyster at room temperature, starting now. Plan grow #2 as
shiitake in the cold half of the year. Buy no cooling until you have a reason to.

---

## 1. The substrate question — fibre vs pellets, and what the science says

### 1.1 What you are actually building

A fruiting substrate is three things in balance:

- **A carbon skeleton** — lignocellulose. Sawdust, wood pellets, straw. This is the bulk.
- **A nitrogen supplement** — bran, soy hulls, seed meal. This is the yield multiplier.
- **Water** — at *field capacity*, ~60–65 % moisture by weight. Nearly everyone's first
  failure is getting this wrong.

More nitrogen means more mushrooms and faster colonisation — right up until it means a bag of
bacteria instead. That trade-off is the entire craft of substrate formulation.

### 1.2 The evidence on supplementation

This is one of the better-studied questions in the literature, and the numbers are striking.
On sawdust substrate, oyster mushrooms supplemented with **15 % wheat bran** yielded 683.9 g per
500 g of substrate — a **biological efficiency of 136.8 %** — significantly beating other
supplementation levels. Colonisation speed rises with supplementation too: 20 % bran fully
colonised in **33 days** against **43 days** at 5 % bran.

> **Biological efficiency (BE)** = fresh mushroom weight ÷ *dry* substrate weight × 100.
> Because fresh mushrooms are ~90 % water, BE above 100 % is normal and expected, not a typo.
> It is the standard yield metric — learn to think in it. Unsupplemented straw runs 40–75 % BE;
> a good supplemented hardwood mix runs 100 %+.

The catch: **every percent of nitrogen you add is also food for *Trichoderma* and bacteria.**
This gives a hard rule that governs your whole workflow:

| Supplementation | Heat treatment required | Equipment |
|---|---|---|
| 0 % (plain straw pellets) | Pasteurisation, 65–80 °C for 1–2 h | Bucket + kettle |
| Up to ~5 % bran | Pasteurisation, carefully | Bucket + kettle |
| **Over ~5 %** | **Full sterilisation, 121 °C / 15 PSI, 2–2.5 h** | **Pressure cooker** |
| Master's Mix (50 % soy hull) | **Full sterilisation, non-negotiable** | **Pressure cooker** |

There is no way around this. A supplemented bag that is only pasteurised is a coin flip, and the
coin is weighted against you.

### 1.3 Master's Mix — the gold standard, and its Swedish problem

The reference formula in gourmet cultivation is **Master's Mix**: 50 % hardwood sawdust/pellets
+ 50 % soy hulls by dry weight, hydrated to ~60 %. Popularised by Mark Rasmussen (T.R. Mushrooms),
it is the default commercial substrate for oyster, lion's mane and king oyster in North America,
and routinely produces BE in the 75–100 %+ range on the first flush alone.

Here is the honest problem: **Master's Mix is awkward and expensive to build in Sweden**, for two
independent reasons.

**Problem 1 — Swedish wood pellets are the wrong wood.** Virtually all bulk *träpellets* sold for
heating in Sweden are **softwood** (spruce and pine — *barrträ*). Conifer resins, terpenes and
phenolics actively inhibit mycelial growth for the gourmet species you care about. A 16 kg sack
of heating pellets from a petrol station is 40 kr and completely useless to you. Do not buy it.
Hardwood (*lövträ*) pellets are a specialty item here, running roughly **30–40 kr/kg**, often
plus freight.

**Problem 2 — pure soy hulls are essentially not retailed in Sweden.** *Sojaskal* exists in the
feed trade only as a component of blended horse feeds. The closest widely available product,
HorseLux FiberPellets (15 kg, Granngården and most feed shops), lists soy hulls and dried beet
pulp — but also **added vegetable oils**, which you do not want in a mushroom substrate. Be aware
also that Granngården's "Soja Pelletskross" is soy *meal*, not hulls: far higher protein, which
means it behaves like an aggressive supplement, not like bulk. Do not substitute it 1:1.

**Conclusion:** treat Master's Mix as the aspirational formula, not the starting one. The Swedish
substrate strategy below gets you most of the yield at a fraction of the cost and hassle.

### 1.4 Substrate options ranked for a Gothenburg grower

| # | Formula | Cost | Heat treatment | Best for | Verdict |
|---|---|---|---|---|---|
| **1** | **Straw pellets, plain** | **~5 kr/kg** | Pasteurise | Oyster, king oyster | **Start here.** Cheapest, easiest, no pressure cooker |
| 2 | Straw pellets + 10–15 % wheat bran | ~6 kr/kg | **Sterilise** | Oyster, all Pleurotus | Big yield jump, needs a pressure cooker |
| 3 | Birch/oak pellets + 15 % bran | ~35 kr/kg | **Sterilise** | Shiitake, Lion's Mane, Maitake | Required for wood-lovers |
| 4 | True Master's Mix (hardwood + soy hull) | ~40 kr/kg | **Sterilise** | Everything | Best yields, worst Swedish availability |
| 5 | Softwood heating pellets | 2 kr/kg | — | **Nothing** | **Do not buy.** Resins inhibit growth |

**The headline cost hack**, and the direct answer to your "cheapest in Gothenburg" question:

> **Straw pellets bought as horse bedding cost about one sixth of straw pellets sold for
> mushroom growing.**
>
> - Svamphuset *halmpellets*, 5 kg — **159 kr** = **31.80 kr/kg**
> - Granngården *Halmpellets Hästströ*, 13 kg — **65 kr** = **~5 kr/kg**
>
> It is the same product: pure Swedish straw, ground, pelleted, heat-treated, no binding agents.
> The horse bedding is arguably *better* documented for our purposes because absorption is a
> selling point (spec'd at ≥5 L water per kg). One 13 kg sack is roughly **39 kg of hydrated
> substrate** — far more than your 65 L chamber will hold at once, so buy one sack and split it
> with someone, or store it dry. A full pallet (60 × 13 kg) is 6 499 kr if you ever go
> commercial.

The same logic applies to oak: **grill pellets**. Pini 15 kg 100 % oak grill pellets on Amazon.se
are food-grade oak with no binders, sold for pizza ovens and smokers, and work as mushroom
substrate at a fraction of the specialty price. Check the bag says 100 % *ek* or *bok* with no
flavour oils added.

### 1.5 Hydration — the step most first grows fail on

Target **60–65 % moisture**. Pellets make this easy because they are bone dry and you can just
measure water in.

Svamphuset's own ratio for straw pellets, which is a good starting point:

> **1 kg dry straw pellets : 1.9 L room-temperature water → ~3 kg hydrated substrate**

For hardwood pellets, use **1 kg : 1.2–1.4 L** — wood holds less water than straw. Add water,
seal, wait 20 minutes for the pellets to burst and fluff, then mix.

**The squeeze test** — the universal field-capacity check, worth more than any scale:

- Squeeze a handful hard.
- **A few drops** run out → correct. Proceed.
- **A stream** → too wet. Anaerobic pockets, bacterial blotch, likely failure. Add dry pellets.
- **Nothing, and it crumbles** → too dry. Colonisation stalls. Add water 100 ml at a time.

Too wet is much worse than too dry, and it is the more common error. When in doubt, err dry.

### 1.6 Optional additions, honestly assessed

- **Gypsum (calcium sulphate), 2 % dry weight** — buffers pH, adds calcium, and keeps pellets
  from clumping into a solid brick. Cheap, mildly helpful, genuinely optional. Skip it on run #1.
- **Hydrated lime** — for cold lime pasteurisation of loose straw. Irrelevant for pellets, which
  are already heat-treated.
- **Coffee grounds** — the classic beginner's "free substrate". They are nitrogen-rich, already
  colonised by competing moulds, and contaminate at a high rate. The internet oversells them.
  If you must, keep them under 20 % of the mix and use them the same day they are brewed.
- **Coir + vermiculite** — the standard bulk substrate in a different corner of the hobby. Poor
  fit for wood-loving gourmet species. Not recommended for your seven.

---

## 2. Spawn — what to actually inoculate with

### 2.1 Grain vs sawdust spawn

| | Grain spawn | Sawdust spawn |
|---|---|---|
| Nutrition | High | Low |
| Colonisation speed | **Fast** | Slower |
| Contamination risk if exposed | High | Low |
| Best into | Straw, supplemented bulk | Wood substrates, logs |
| Swedish availability | Limited for gourmet species | **Good** |

For your use case — pellet-based bulk substrate in bags — **grain spawn colonises faster**, and
speed is your main defence against contamination. But Swedish retail for gourmet grain spawn is
thin, so sawdust spawn is what you will realistically buy first. It works fine; add a week.

### 2.2 Spawn rate — the cheapest reliability you can buy

Spawn rate is the percentage of spawn to bulk substrate by weight. The relationship is simple and
worth internalising:

**Higher spawn rate → faster colonisation → less time exposed → fewer contaminated bags.**

- **5 %** — Svamphuset's stated ratio (150 g spawn into ~3 kg substrate). Economical, slower.
- **10 %** — a good general-purpose rate.
- **15–20 %** — what to use if you have had contamination problems, or on your very first grow.

If a grow fails at 5 % and succeeds at 15 %, the extra spawn was by far the cheapest variable you
could have changed. Do not be frugal with spawn on run #1.

### 2.3 Swedish spawn suppliers

| Supplier | Species relevant to you | Format | Price |
|---|---|---|---|
| **[Svamphuset](https://svamphuset.com)** | Oyster, Shiitake, Lion's Mane, King Oyster, Shimeji | Sawdust spawn | **from 199 kr** |
| Svamphuset | Wine Cap, Parasol | Grain spawn, 1 kg | 549–569 kr |
| **[Skymnäs Svamp](https://skymnassvamp.se)** | Oyster, blue oyster | Grain spawn 500 g, liquid culture | Check site |
| [Svamperiet](https://www.svamperiet.se/butik) | Various | Spawn + substrate | Check site |
| [Tunnelväxthus](https://tunnelvaxthus.se) | Grey oyster (Svamphuset stock) | Mycelium 1 kg | Check site |

Svamphuset grows on certified organic Swedish wheat and covers five of your seven firmware
species — they are the practical default. Skymnäs is smaller and publishes a genuinely good
Swedish-language growing guide worth reading alongside this one.

### 2.4 The big cost saver: make your own grain spawn

At 549 kr/kg retail, spawn is the most expensive consumable in the hobby. Rye berries
(*rågkärnor*) from a Swedish supermarket cost roughly 20–30 kr/kg. Once you have a pressure
cooker and can work cleanly, you can expand one bought bag of spawn into many kilos.

The standard method, in outline:

1. Rinse rye, then **soak 12–24 h**.
2. Simmer 15–20 min until grains are hydrated but **not split**. A split grain is a dead grain.
3. Drain thoroughly and surface-dry — wet grain surfaces breed bacteria.
4. Fill jars or filter-patch bags to ~⅔, leaving shake room.
5. **Sterilise 90–120 min at 15 PSI.** Cool fully — overnight.
6. Inoculate in still air; shake to distribute once ~25 % colonised.

**Grain-to-grain (G2G) transfer** is the multiplier: one colonised jar inoculates 5–10 more,
each colonising in about a week. This is how 199 kr of spawn becomes a year of growing. It is
also the step that most demands real sterile technique — attempt it only after a clean first grow.

**Do not endlessly re-clone.** Mycelium **senesces** — vigour degrades over successive transfers.
Keep transfers from a single purchase to roughly 3–4 generations, then buy fresh genetics.

### 2.5 Liquid culture

Liquid culture (mycelium in sterile sugar solution) is cheap, fast, and lets you inoculate grain
by syringe through a self-healing port. Svamphuset sells LC. It is an excellent second-year
technique — it demands more sterile discipline than sawdust spawn, and contamination is harder to
spot in a cloudy liquid. Note it and move on for now.

---

## 3. Sterilisation and contamination control

### 3.1 Pasteurisation vs sterilisation

- **Pasteurisation (65–80 °C, 1–2 h)** knocks competitors back and leaves some beneficial
  microflora that actively resists invaders. Enough for *unsupplemented* straw pellets.
- **Sterilisation (121 °C / 15 PSI, 2–2.5 h)** kills everything, including bacterial
  endospores. Leaves a nutritional vacuum — whatever lands first, wins. Mandatory for anything
  supplemented, and it means the inoculation must be genuinely clean.

**The bucket-pasteurisation method** (no equipment beyond a kettle), which is all you need for
oyster on straw pellets:

1. Weigh dry pellets into a heat-safe bucket or directly into your grow bag.
2. Pour on **boiling** water at 1.9 L per kg.
3. Seal or cover immediately, wrap in a towel or duvet.
4. Leave 4–12 h (overnight is easiest). The pellets absorb everything and the mass holds
   pasteurisation temperature for hours.
5. Cool to **under 25 °C** before adding spawn. **Warm substrate kills mycelium** — this is a
   real and common failure. Use a thermometer, not a guess.

Straw pellets are already heat-treated during manufacture, so this is closer to a re-pasteurisation
than a rescue of raw field straw. It is the reason pellets are so much more reliable than baled straw.

### 3.2 The pressure cooker decision

You need one for: supplemented substrate, grain spawn, shiitake, lion's mane, maitake — i.e.
everything past the beginner tier.

Requirements: a genuine **pressure cooker** (*tryckkokare*) reaching 15 PSI / 121 °C, not a
pressure *saucepan* that only reaches 5–10 PSI. Minimum useful size is ~10 L for jars; 23 L is
the hobbyist standard and fits bags. An All American 921 is the lifetime buy at a painful price;
a large Swedish/Nordic tryckkokare from a kitchen retailer works fine. Watch Blocket and Facebook
Marketplace — canning fell out of fashion and large pressure cookers turn up cheaply.

**Budget 800–2500 kr** and consider it grow #2's purchase, not grow #1's.

### 3.3 The still-air box — a second SAMLA box, ~150 kr

A **SAB** is a clear plastic tub, inverted, with two arm holes cut in one side. You work inside
it. It has no fan and no filter; the entire principle is that **still air does not carry spores**.
It costs almost nothing and eliminates the majority of beginner contamination.

- A second **IKEA SAMLA** (the 45 L or 65 L, same as your chamber) is the ideal donor.
- Cut two 15 cm holes, low, angled so you work comfortably.
- Wipe the interior with 70 % isopropanol, let it settle **10 minutes** before opening anything.
- Move slowly. Never pass your hands *over* an open bag or jar.

Do not buy a laminar flow hood. It is a 5000+ kr answer to a problem a 150 kr box solves at your
scale.

### 3.4 Sterile technique, condensed

- **70 % isopropanol, not 99 %.** 70 % penetrates cell walls and kills better; 99 % flash-evaporates.
  Buy it at Apoteket or a paint shop (*T-röd* is denatured ethanol and also works).
- Work in a **clean, still room**. No open windows, no fans, no pets, no vacuuming beforehand.
  Ideally a bathroom just after a hot shower — steam has pulled spores out of the air.
- **Wipe everything**: hands, forearms, bag exteriors, jar lids, scissors, the table.
- **Flame-sterilise** metal to glowing red, cool 15 s before contact.
- Open bags for the **shortest possible time**. Have everything laid out before you start.
- Long sleeves, tied-back hair. Skin sheds constantly.

### 3.5 Reading contamination

| Appearance | Almost certainly | Action |
|---|---|---|
| White, fluffy, spreading from spawn | **Healthy mycelium** | None |
| Blue-green powder, sharp musty smell | ***Trichoderma*** (green mould) | **Bag out, sealed, in the bin. Do not open indoors** |
| Black pinhead dots | *Aspergillus* / *Rhizopus* | Discard. **Genuine respiratory hazard** |
| Wet, slimy, sour or rotten smell | Bacterial blotch — substrate too wet | Discard; reduce hydration next time |
| Orange/pink slime | Bacterial | Discard |
| Blue/green *staining* on healthy mycelium | Bruising (normal in Pleurotus) | None |
| Yellow-orange droplets on mycelium | **Metabolite exudate — normal**, often a pinning precursor | None |

The two that fool people constantly: **exudate droplets** and **bruising** are both healthy and
routinely thrown out by nervous beginners. Conversely, *Trichoderma* starts white and only turns
green once it sporulates — by which time it is established. White mould advancing on a *front*
rather than radiating from a spawn point deserves suspicion.

**When you find green mould, do not open the bag indoors.** You will aerosolise millions of
spores into the room you grow in. Seal it, take it outside, bin it.

---

## 4. Fruiting in a 65 L SAMLA

### 4.1 Capacity

A 65 L SAMLA (~78 × 56 × 18 cm) is a wide, shallow footprint. Realistically it holds:

- **2–3 bags of ~2 kg** standing upright, or
- **3–4 blocks of ~1.2 kg**, or
- one shallow bulk tray

At 10 % spawn and ~75 % BE, three 2 kg blocks give roughly **1.3–1.5 kg of fresh oysters per
flush**, across 2–3 flushes. That is a lot of mushrooms for a household. Do not overfill — the
shallow lid height is your real constraint, and oyster clusters need vertical room.

### 4.2 Fresh air exchange — the thing your build under-serves

Mushrooms are aerobic and exhale CO₂. Accumulated CO₂ causes the classic failure mode: **long
stringy stems, tiny or absent caps** ("leggy" fruits). Oyster wants CO₂ **under ~800 ppm** during
fruiting; a sealed 65 L box with 2 kg of fruiting mycelium blows past that in under an hour.

Your two 40 mm fans plus filter discs will move air, but note that your controller ventilates on
a **humidity-driven** state machine (`HUMIDIFYING → STABILIZING → VENTILATING → RECOVERING`), not
a CO₂-driven one. In a well-sealed humid box those can decouple: humidity may sit perfectly on
target while CO₂ climbs unchecked.

Two cheap fixes, no new firmware required:

- **Passive**: 4–6 holes of 15–25 mm in the sides, covered with **synthetic filter discs** (BOM
  item 15) or micropore tape. Passive exchange plus your circulation fans is often enough at this
  scale, and it is the standard shotgun-fruiting-chamber approach.
- **Diagnostic**: an **MH-Z19B NDIR CO₂ sensor** (~150–250 kr) tells you whether you have a
  problem at all, instead of guessing. Even used purely as a readout, it is the highest-value
  ~200 kr you can add to this build.

The symptom to watch for: long stems, small caps → too much CO₂, ventilate more.

### 4.3 Light

Mushrooms do not photosynthesise. Light is a **morphological signal** — it triggers pinning and
tells fruits which way is up. Your WS2812B strips at ~6500 K blue-white are correct, and your
config's 8–12 h schedules are right. Intensity requirements are low (500–1000 lux); anything that
reads as "comfortable indoor daylight" works. Do not add more light thinking it adds yield.

> Note the dependency chain your firmware already documents: `controlLighting()` needs NTP.
> If time sync fails, the schedule is meaningless. Worth a dashboard indicator.

### 4.4 Harvest

- **Oyster**: harvest when cap edges are still slightly curled **down**. Once they flatten and
  turn up, the cluster starts dropping spores — spores foul your filters, irritate lungs, and
  the mushrooms lose texture. Twist the whole cluster off at the base; do not cut and leave stumps.
- **Shiitake**: harvest at 60–70 % cap opening, before the veil fully tears.
- **Lion's Mane**: harvest when the spines are 1 cm+ but before it yellows. Yellowing means past
  peak and it turns bitter.

**Between flushes**: after harvest, the block rests. Some growers soak blocks in cold water for
several hours to rehydrate before flush #2. Expect flush #1 ≈ 50–60 % of total yield, flush #2
≈ 25–30 %, flush #3 the remainder. Three flushes is a normal life.

---

## 5. Shopping lists

### Tier 1 — Minimum viable first grow (oyster, no pressure cooker)

| Item | Where | Price |
|---|---|---|
| Halmpellets Hästströ 13 kg | **Granngården** | **65 kr** |
| Oyster sawdust spawn | Svamphuset | from 199 kr |
| Grow bags | **already owned** | — |
| 70 % isopropanol / T-röd | Apoteket / paint shop | ~60 kr |
| Digital thermometer | Biltema / Clas Ohlson | ~80 kr |
| Second SAMLA for a still-air box | IKEA | ~100 kr |
| Micropore tape | Apoteket | ~40 kr |
| **Total** | | **~550 kr** |

That is a complete, credible first grow for the price of a takeaway dinner for two. It should
yield 1–1.5 kg of oysters.

### Tier 2 — Serious setup (add for supplemented substrate and shiitake)

| Item | Where | Price |
|---|---|---|
| Pressure cooker, 23 L, 15 PSI | Blocket / kitchen retailer | 800–2500 kr |
| Wheat bran (*vetekli*) | Granngården / supermarket | ~30 kr/kg |
| Oak grill pellets 15 kg (100 % ek) | Amazon.se / grill shop | ~250 kr |
| Rye berries (*rågkärnor*) for own spawn | ICA Maxi / Coop | ~25 kr/kg |
| Wide-mouth jars for grain spawn | IKEA / Rusta | ~25 kr each |
| MH-Z19B CO₂ sensor | AliExpress | ~200 kr |
| Gypsum, optional | Garden centre | ~50 kr |

### Tier 3 — Only if you continue

Scale, agar work (petri dishes, malt extract agar) for isolating and preserving genetics, a
second chamber so incubation and fruiting run in parallel, cooling for cold-fruiting species.

---

## 6. Gothenburg sourcing map

| What | Where | Notes |
|---|---|---|
| **Straw pellets, wheat bran, feed** | **Granngården**, incl. Backaplan | The cost hack. Check [granngarden.se/butiker](https://www.granngarden.se/butiker) for current hours |
| Spawn (5 of your 7 species) | [Svamphuset](https://svamphuset.com), online | Swedish, organic grain, fast shipping |
| Spawn + Swedish growing guide | [Skymnäs Svamp](https://skymnassvamp.se) | Good Swedish-language *Odlingsguiden* |
| Substrate + accessories | [Svamperiet](https://www.svamperiet.se/butik) | Compare before ordering |
| Oak grill pellets | Amazon.se, grill retailers | Verify 100 % ek/bok, no flavour oils |
| SAMLA boxes | IKEA Bäckebol / Kållered | Chamber + still-air box |
| Isopropanol / T-röd | Apoteket, paint shops | 70 %, not 99 % |
| Thermometers, tubing, fans | Biltema, Clas Ohlson, Jula | |
| Rye berries, jars | ICA Maxi, Coop, Willys | Saltå Kvarn stocks rye |
| Used pressure cookers, wine fridges | Blocket, FB Marketplace | Patience pays here |

---

## 7. First grow — an 8-week timeline

**Week 0 — prep**
Change firmware to `OYSTER`. Order spawn. Buy a sack of straw pellets. Build the still-air box.
Confirm the chamber holds 85–90 % RH with nothing in it — debug the hardware on an empty box,
not on a live grow.

**Week 1 — inoculate**
Weigh pellets into the bag. Boiling water at 1.9 L/kg. Seal, wrap, leave overnight. Next day
confirm **under 25 °C**, then add spawn at **10–15 %** inside the SAB. Mix thoroughly, seal, tape
the filter patch.

**Weeks 1–3 — incubation**
Set phase to `Incubation`. **24 °C, 70 % RH, dark.** A warm cupboard beats the chamber here — the
chamber is more useful for fruiting. Check for contamination every few days; do not open the bag.
Expect full white colonisation in 14–21 days.

**Week 3–4 — consolidation and pinning**
Let it go fully white, then **3–5 more days** to consolidate. Impatience here is the second most
common failure after over-hydration. Then set phase to `Primordia`: **13 °C if you can manage it,
93 % RH, light on**. Cut a slit or X in the bag. Pins appear in 3–7 days.

**Week 5–6 — fruiting**
Set phase to `Fruiting`: **18 °C, 88 % RH, 8–12 h light, maximum fresh air.** Fruits double daily.
Harvest when cap edges are still curled down — typically 5–7 days from pinning.

**Weeks 6–8 — further flushes**
Rest, optionally soak the block, return to fruiting conditions. Expect 2–3 flushes total, then
compost the spent block (it makes excellent garden mulch, and often fruits once more outdoors).

---

## 8. Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| Long stems, tiny caps | **CO₂ too high** | More fresh air exchange — the #1 chamber-grow problem |
| No pins after full colonisation | No trigger | Cold shock, raise humidity, ensure light, cut the bag |
| Fuzzy white "mould" on pins | **Usually healthy aerial mycelium** | Raise humidity, increase FAE. Rarely a real problem |
| Green mould | *Trichoderma* | Discard sealed, **outdoors**. Review sterile technique |
| Sour smell, slimy | Substrate too wet | Discard; less water next time |
| Colonisation stalls | Too cold / too dry / weak spawn | Check temp is 24 °C; check hydration |
| Dry, cracked cap surfaces | Humidity too low | Check humidifier and RH sensor calibration |
| Mushrooms abort, go yellow/brown | Humidity crash or CO₂ spike | Check the humidity state machine is cycling |
| Tiny first flush | Under-supplemented or under-spawned | Raise spawn rate; supplement and sterilise |

---

## 9. Cost reality check

Rough per-kilo cost of fresh oyster mushrooms, at 75 % BE, one 2 kg block:

| Line | Cost |
|---|---|
| Straw pellets (~0.65 kg dry) | ~3 kr |
| Spawn at 10 % (amortised, bought) | ~30 kr |
| Electricity, water | ~5 kr |
| **Per block (~1.5 kg fresh over all flushes)** | **~38 kr** |
| **Per kg fresh** | **~25 kr/kg** |

Gourmet oyster mushrooms retail in Gothenburg at roughly 200–400 kr/kg. Once you make your own
grain spawn, the marginal cost drops under 10 kr/kg. The economics are genuinely good — the
capital cost is the chamber, and you have already built that.

---

## 10. Sources

Swedish suppliers and pricing:
- [Svamphuset — substrates](https://svamphuset.com/en/collections/substrates), [rapid mycelium](https://svamphuset.com/en/collections/rapid-mycelium), [straw pellets](https://svamphuset.com/en/products/straw-pellets), [oak pellets](https://svamphuset.com/en/products/oak-pellets)
- [Granngården — Halmpellets Hästströ 13 kg](https://www.granngarden.se/halmpellets-haststro-13kg) and [halmpellets category](https://www.granngarden.se/lantbruk-skog/stromedel/halmpellets)
- [Granngården — store finder](https://www.granngarden.se/butiker)
- [Skymnäs Svamp — Odlingsguiden](https://skymnassvamp.se/sidor/odlingsguiden)
- [Svamperiet](https://www.svamperiet.se/butik)
- [HorseLux FiberPellets 15 kg](https://www.granngarden.se/hastfoder-horselux-fiberpellets-15kg) (soy hull content)
- [Byggahus — Lövträpellets i Sverige?](https://www.byggahus.se/forum/threads/lovtrapellets-i-sverige.482728/) (hardwood pellet availability and pricing)
- [Pini 100 % oak grill pellets, Amazon.se](https://www.amazon.se/Pini-grillpellets-tr%C3%A4pellets-pelletdrivna-pizzaugnar/dp/B07V25MTNC)

Substrate science:
- [Effect of wheat bran supplement on growth and yield of oyster mushroom on sawdust substrate](https://www.researchgate.net/publication/337740556_Experimental_Agriculture_Horticulture_Effect_of_Wheat_Bran_Supplement_on_Growth_and_Yield_of_Oyster_Mushroom_Pleurotus_Ostreatus_on_Fermented_Pine_Sawdust_Substrate) — the 15 % bran / 136.8 % BE and colonisation-speed figures
- [GroCycle — What is Master's Mix](https://grocycle.com/what-is-masters-mix/)
- [FreshCap — Growing Mushrooms on Soy Hulls (Master's Mix)](https://learn.freshcap.com/growing/growing-mushrooms-on-soy-hulls-the-masters-mix/)
- [Alfalfa pulp as a substrate for oyster mushroom cultivation (NIH/PMC)](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC9407111/)
- [Sara Bäckmo — Odla svamp i pallkrage](https://sarabackmo.se/odla-svamp-i-pallkrage-ostronskivling/) (Swedish straw-pellet practice)

Standard references worth owning: Paul Stamets, *Growing Gourmet and Medicinal Mushrooms*;
Tradd Cotter, *Organic Mushroom Farming and Mycoremediation*.
