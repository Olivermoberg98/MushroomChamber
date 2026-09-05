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

> **Done** — `main.cpp` now calls `getMushroomConfig(OYSTER)`.

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

**The Swedish workaround: beech grill pellets + wheat bran.** Both of Master's Mix's Swedish
problems are sourcing problems, and grill pellets solve the first one — 100 % *bok* or *ek* sold
food-grade for pizza ovens is real hardwood at 17–25 kr/kg, not the 30–40 kr/kg specialty price,
and nothing like the useless softwood *träpellets* from a petrol station. Pair it with 15 % wheat
bran instead of unobtainable soy hulls and you have a substrate in the same class, from two
ingredients you can actually buy here. **It needs a pressure cooker** — 15 % bran is well past the
pasteurisation ceiling in §1.2 — so it is the formula to graduate to, not to start on. See §1.7
for the unsupplemented, pasteurise-only version you can run today.

### 1.4 Substrate options ranked for a Gothenburg grower

| # | Formula | Cost | Heat treatment | Best for | Verdict |
|---|---|---|---|---|---|
| **1** | **Hardwood pellets, plain (beech / alder)** | **~25 kr/kg** | Pasteurise | **All seven.** Start here | **The no-pressure-cooker default.** No added N, so pasteurising is safe |
| 1b | Straw pellets, plain | ~5 kr/kg *by the sack* | Pasteurise | Oyster, king oyster | Faster for *Pleurotus*, but see the pack-size trap in §1.7 |
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
>
> **The catch: the 13 kg sack is a *butiksvara*.** Granngården sells it in store only — online
> you can order a 60-sack pallet or a bigbag, nothing between. Börjes Hästsport carries the same
> product at 95 kr and is likewise "säljs endast i butik". Neither has a Göteborg store: nearest
> Granngården is **Kungälv or Kungsbacka**, nearest Börjes is **Kungsbacka**.
>
> **So the 5 kr/kg never applies to a hobbyist.** Priced in pack sizes you can actually buy in the
> city, straw is the 1 kg Fresh Fungi bag at Hornbach — **65 kr/kg**, more than twice the price of
> beech grill pellets. The cost hack is real only if you are buying by the sack and will drive for
> it. See §1.7.

The same logic applies to oak: **grill pellets**. Pini 15 kg 100 % oak grill pellets on Amazon.se
are food-grade oak with no binders, sold for pizza ovens and smokers, and work as mushroom
substrate at a fraction of the specialty price. Check the bag is **100 % lövträ with 0 % fillers**
and no flavour oils. A *blend* of hardwoods is fine — bok/al, bok/ek — what matters is that no
conifer (*barrträ*) is in it.

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

### 1.7 Run #1, as actually received — oyster, lion's mane and shiitake

Spawn in hand, 1 September 2026: **220 g Oyster, 220 g Lion's Mane, 280 g Shiitake.** That fixes the
substrate quantities, because spawn rate is the input you size everything else from.

**One substrate for both species: Weber hardwood pellets.**

> **[Weber Alpellets, 8 kg — ~200 kr](https://www.weber.com/IE/en/accessories/fuel/wood-pellets/18290.html)**
> (Amazon.se). **40 % al (alder) / 60 % bok (beech), 0 % fillers**, FSC, EU-made — clears the §1.4
> test. **~25 kr/kg.** Weber's [pure bok pellets](https://www.weber.com/GB/en/accessories/18292.html)
> are equivalent if the alder blend is out of stock.
>
> Both woods are right for your species. Beech is the classic: lion's mane and shiitake grow on it
> in the wild, and it lacks oak's tannins, which slow colonisation slightly. **Alder is a genuine
> hardwood** — a broadleaf in the birch family, no conifer resins — and the beech/alder pairing is
> one of the standard recommendations for oyster and lion's mane specifically.
>
> **What the alder changes, and it is not much:** alder is less dense than beech (~450–500 against
> ~720 kg/m³), so it breaks down faster — expect slightly quicker colonisation and slightly fewer
> late flushes, with the first flush unaffected. Alder also fixes nitrogen (*Frankia* symbiosis) so
> its wood carries a little more N than most hardwoods; at 40 % of a blend that is nowhere near
> bran territory, but it nudges the substrate a hair toward "more nutritious, marginally more
> contamination-friendly." Not enough to change the protocol — just no excuse for slack technique
> on the supplemented bags.
>
> Keep hydration at **1.2–1.4 L/kg** and let the squeeze test decide. Less dense wood takes a touch
> more water, so aim mid-range rather than low. 8 kg covers this grow eight times over.

**Why not straw for the oyster.** §1.4's cost hack says straw is 5 kr/kg against hardwood's 25 —
but that price is the **13 kg horse-bedding sack**, an in-store-only *butiksvara* from a shop
outside Göteborg. Priced as you can actually buy it in the city, straw is the **1 kg Fresh Fungi
bag at Hornbach: 65 kr/kg**, which is *two and a half times the price of the hardwood*. The cost
argument for straw inverts completely at the pack sizes available to a one-grow hobbyist.

What you give up is small: *Pleurotus ostreatus* is the straw specialist of its genus and
colonises straw somewhat faster. Against that, oyster on hardwood is entirely standard — Master's
Mix (§1.3) is hardwood-based and oyster is its main commercial crop. And running one substrate
means **one hydration ratio**, which removes a real failure mode: straw takes 1.9 L/kg and wood
takes 1.2–1.4, and pouring straw's ratio onto wood gives a sodden anaerobic block.

### Quantities

**Seven bags.** Oyster and lion's mane split three ways each across a bran ladder (0 / 5 / 10 %) at
600 g of pellets per bag; shiitake runs as a single small bag at a proper spawn rate. Full table and
reasoning in §7.0.

| | Oyster & Lion's Mane, each of 6 | Shiitake, 1 bag |
|---|---|---|
| Dry pellets | **600 g** | **600 g** |
| Vetekli | 0 / 30 / 60 g | **none** |
| Boiling water (1.3 L/kg total dry) | 780 / 820 / 860 ml | **780 ml** |
| Block | ~1 380–1 520 g | ~1 380 g |
| Spawn | **73 g** | **280 g** |
| Spawn rate | **~5 %** | **~20 %** |

Run #1 therefore uses **4 200 g of the 8 kg bag** and 180 g of vetekli. Every bag takes the same
600 g of pellets — only the water and the bran differ.

The two spawn rates are a deliberate split. Five percent on the oyster and lion's mane is a chosen
trade — slower colonisation, more bags, more of the ladder tested — and §7.0 lists what it costs and
the two free mitigations that buy some of it back. The shiitake gets 20 % because its incubation
runs 8–16 weeks, and the bag with the longest exposure is the one that can least afford a thin
spawn rate.

### On the bran

The 90/10 pellets/vetekli mix you first considered is above what boiling water can carry — 10 % is
double the pasteurisation ceiling in §1.2. Rather than guess where the real ceiling sits in your
kitchen, run #1 **tests all three points**: 0 % as control, 5 % at the documented ceiling, and 10 %
deliberately past it, on both oyster and lion's mane.

Three bags per species is what makes that affordable. A failure costs one bag out of three, the
control guarantees you still grow the species, and the grow log turns a guess into a number for run
#2. Full protocol in §7.

### Hardwood as the house substrate

A sound long-term default. Shiitake is traditionally grown on beech, *buna*-shimeji literally means
"beech mushroom", and lion's mane, enoki, maitake and king oyster all take it — as they do alder.
Oyster is the only
one of your seven with a meaningfully better alternative, and only if you can buy straw by the sack.

Two limits worth keeping in view:

- **Hardwood does not replace the pressure cooker.** For shiitake, maitake, king oyster and enoki the
  binding constraint was never which wood — it is the ~15 % bran those species need to yield
  properly, and that needs 121 °C.
- **Buy it in bulk once it is the default.** 25 kr/kg is trial-size pricing; 100 % *bok* or *ek*
  sold in 30 kg drops that substantially.

Oyster and lion's mane can fruit in the chamber **at the same time under the current `OYSTER`
config** — oyster at 15–21 °C / 88 % RH, lion's mane at 15–20 °C / 88 %, with the same 8–12 h light
window. **Shiitake cannot join them**: 13 °C / 75 % RH is genuinely incompatible, so it needs the
chamber to itself and a firmware change when its turn comes, months later (§7.8).

Incubation happens in a closet, not the chamber (§7.4), so none of this matters until the first
block is fully colonised. Note also §4.1: the SAMLA holds three or four blocks of this size, not
six — §7.5 covers staggering them.

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
any *unsupplemented* pellet substrate:

1. Weigh dry pellets into a heat-safe bucket or directly into your grow bag.
2. Pour on **boiling** water: **1.2–1.4 L per kg for hardwood**, 1.9 L per kg for straw. The two
   are not interchangeable — straw's ratio on wood gives a sodden, anaerobic block.
3. Seal or cover immediately, wrap in a towel or duvet.
4. Leave 4–12 h (overnight is easiest). The pellets absorb everything and the mass holds
   pasteurisation temperature for hours.
5. Cool to **under 25 °C** before adding spawn. **Warm substrate kills mycelium** — this is a
   real and common failure. Use a thermometer, not a guess.

Pellets of either kind are already heat-treated during manufacture, so this is closer to a
re-pasteurisation than a rescue of raw field straw. It is the reason pellets are so much more
reliable than baled straw.

**There is a third option between the two.** *Tyndallisation* — repeated heat cycles spaced ~24 h
apart, so endospores that survive one cycle germinate and are killed by the next — gets much of the
way to sterilisation with a stockpot instead of a pressure cooker. It is slow and it is not a
guarantee, but it is the honest answer for anyone supplementing without 121 °C. Method in §7.2.

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

#### If you do not have a box yet

Two things carry most of the SAB's benefit and cost nothing.

**1. Never put your hands inside a bag.** This is the big one, and it makes the box far less
critical than it sounds:

- Wipe the outside of the spawn bag with 70 % isopropanol and **cut a corner off it**.
- Open the substrate bag **just enough to pour through** — a gap, not a mouth. Pour the spawn in.
- Close it immediately, then **mix by kneading the sealed bag from the outside.** Spawn distributes
  through the plastic just as well as it does with your hands in it.

Open time falls from minutes to seconds per bag, nothing on your skin ever enters, and you never
have to pass your hands over an open bag because you never open one properly.

**2. Work in a steamed, settled bathroom.** A small room with no through-draught is the next best
thing to a box, and steam actively cleans the air — water droplets nucleate on dust and spores and
carry them to the floor.

1. Clear the room. Wipe the work surface with 70 % isopropanol.
2. **Run the hot shower hard for ~10 minutes**, door shut, extractor fan **off**.
3. Turn it off, leave, and let the room sit **20–30 minutes** with the door closed. This is the
   settling step and skipping it wastes the whole exercise.
4. Meanwhile wash your arms to the elbows, put on clean clothes, tie hair back. A face mask is
   worth wearing; if you have none, simply do not talk or breathe over the work.
5. Re-enter **slowly**. Every quick movement re-suspends what just settled.
6. Lay a fresh bin bag or clean towel as your work surface — the room will be wet, and you want
   nothing dripping into a bag. Wipe hands with 70 % IPA and let them air-dry.
7. Work low, close to the surface, one bag fully closed before the next is opened.

**Improvised boxes**, if you have any of these already: any clear plastic storage tub inverted (it
does not have to be a SAMLA); a cardboard box laid on its side, working through the open face; or a
large transparent bag used as a glove bag, with both the substrate and the spawn inside it.

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
| Weber lövträpellets 8 kg (alpellets 40/60, or 100 % bok) — **all seven bags** | Amazon.se | **~201 kr** |
| Vetekli, for the 5 % bags (§7.0) | Granngården / supermarket | ~30 kr |
| Kitchen scale reading 1 g | Clas Ohlson / IKEA | ~150 kr |
| Oyster *snabbväxande mycel* 220 g | Svamphuset | from 199 kr |
| Lion's Mane *snabbväxande mycel* 220 g | Svamphuset | from 199 kr |
| Shiitake *snabbväxande mycel* 280 g | Svamphuset | from 199 kr |
| Grow bags | **already owned** | — |
| 70 % isopropanol / T-röd | Apoteket / paint shop | ~60 kr |
| Digital thermometer | Biltema / Clas Ohlson | ~80 kr |
| Second SAMLA for a still-air box (or any clear tub) | IKEA / Rusta / Jula | ~50–100 kr |
| Micropore tape | Apoteket | ~40 kr |
| **Total** | | **~1 265 kr** |

That is a complete, credible first grow for the price of a takeaway dinner for two. The oyster bag
should yield 1–1.5 kg; treat the lion's mane bag as a bonus, for the reasons in §1.7.

### Tier 2 — Serious setup (add for supplemented substrate and shiitake)

| Item | Where | Price |
|---|---|---|
| Pressure cooker, 23 L, 15 PSI | Blocket / kitchen retailer | 800–2500 kr |
| Wheat bran (*vetekli*) | Granngården / supermarket | ~30 kr/kg |
| Oak/beech grill pellets in bulk (100 % ek/bok) | Amazon.se / grill shop | ~17–25 kr/kg |
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
| **Substrate — the house default** | **Weber lövträpellets 8 kg, Amazon.se** | **Start here.** Alpellets (40 % al / 60 % bok) or 100 % bok. ~25 kr/kg, covers every species. See §1.7 |
| Straw pellets, 1 kg, in Göteborg | [HORNBACH](https://www.hornbach.se/varuhusinfo/hornbach-goteborg/), Minelundsvägen 8 | Fresh Fungi, 65 kr/kg. Convenient but the dearest substrate per kg |
| Straw pellets, 13 kg sack | Granngården / Börjes, Kungälv & Kungsbacka | 5 kr/kg, but **no Göteborg store** and in-store only |
| Straw pellets, small bags, shipped | [Svamperiet](https://www.svamperiet.se/butik), [Svamphuset](https://svamphuset.com) | 1–5 kg delivered |
| Wheat bran (*vetekli*), feed | Granngården, supermarkets | Grow #2 ingredient — needs a pressure cooker first |
| Straw pellets, local feed shop | Tollans Häst & Foder, Aspenvägen 11, Härryda | Feed + ridsport, stocks strö. Small shop — phone ahead |
| Spawn (5 of your 7 species) | [Svamphuset](https://svamphuset.com), online | Swedish, organic grain, fast shipping |
| Spawn + Swedish growing guide | [Skymnäs Svamp](https://skymnassvamp.se) | Good Swedish-language *Odlingsguiden* |
| Substrate + accessories | [Svamperiet](https://www.svamperiet.se/butik) | Compare before ordering |
| Hardwood pellets (beech/alder/oak) | Amazon.se, grill retailers | Verify **100 % lövträ, 0 % fillers** — blends are fine, conifers are not. Weber alpellets and bok both confirmed clean |
| SAMLA boxes | IKEA Bäckebol / Kållered | Chamber + still-air box |
| Isopropanol / T-röd | Apoteket, paint shops | 70 %, not 99 % |
| Thermometers, tubing, fans | Biltema, Clas Ohlson, Jula | |
| Rye berries, jars | ICA Maxi, Coop, Willys | Saltå Kvarn stocks rye |
| Used pressure cookers, wine fridges | Blocket, FB Marketplace | Patience pays here |

---

## 7. Run #1 — the exact protocol

Seven bags, three species, one substrate. Written for exactly what is on the table on
**1 September 2026**: 220 g oyster spawn, 220 g lion's mane, 280 g shiitake, an 8 kg bag of Weber
alpellets, vetekli, a kettle, and no pressure cooker.

### 7.0 The seven bags

Oyster and lion's mane each split three ways across a **bran ladder** — 0 %, 5 %, 10 % — so one
grow answers the supplementation question at three points instead of guessing. Shiitake runs alone,
plain, on its own timeline.

| Bag | Species | Bran | Pellets | Vetekli | Water | Block | Spawn | Rate |
|---|---|---|---|---|---|---|---|---|
| **O1** | Oyster | 0 % | 600 g | — | 780 ml | ~1 380 g | 73 g | 5.3 % |
| **O2** | Oyster | 5 % | 600 g | 30 g | 820 ml | ~1 450 g | 73 g | 5.0 % |
| **O3** | Oyster | 10 % | 600 g | 60 g | 860 ml | ~1 520 g | 73 g | 4.8 % |
| **L1** | Lion's Mane | 0 % | 600 g | — | 780 ml | ~1 380 g | 73 g | 5.3 % |
| **L2** | Lion's Mane | 5 % | 600 g | 30 g | 820 ml | ~1 450 g | 73 g | 5.0 % |
| **L3** | Lion's Mane | 10 % | 600 g | 60 g | 860 ml | ~1 520 g | 73 g | 4.8 % |
| **S1** | Shiitake | **0 %** | 600 g | — | 780 ml | ~1 380 g | **280 g** | **20.3 %** |

**Totals:** 4 200 g of pellets (just over half the 8 kg bag), 180 g of vetekli, all 718 g of spawn.
Every bag takes **600 g of pellets** — only the water and the bran differ.

Bran percentages are of *pellet* weight. Water is **1.3 L per kg of total dry matter**, pellets
plus bran — which is why the supplemented bags take slightly more.

#### Why the shiitake is different

Shiitake gets **no bran and a 20 % spawn rate**, and that is deliberate. It is the hardest of your
seven species (§0.1), and its incubation runs **8–16 weeks** against the oyster's few weeks. Every
extra week is another week of exposure, so the one bag where contamination has the most time to find
a way in is the one bag that gets a proper spawn rate and no nitrogen to feed on.

**Use all 280 g of the shiitake spawn on one full-size bag.** Spawn rate is exactly the lever that
counters a long incubation, and shiitake spawn has no alternative use — it cannot go into an oyster
bag, and it does not store well. Holding some back saves nothing. The extra pellets cost about
10 kr, the effort is identical (it is one bag either way), and 280 g into a 1 380 g block is a
genuine 20 % rate rather than a token one. If it works, you want a block worth harvesting.

#### On the 5 % spawn rate

You chose 5 % on the oyster and lion's mane, knowing it is slow. Two consequences to plan around
rather than argue about:

- **It roughly doubles colonisation time.** Timelines in §7.4 already reflect this.
- **It compounds with the bran.** Low spawn means a long exposure window; bran means something worth
  contaminating. O3 and L3 at 10 % bran are the two bags carrying both risks at once — expect them
  to be the ones that fail, and treat that as the experiment working, not as a disaster.

Two things partly buy the penalty back, and both are free:

- **Thermal mass.** With no stockpot, the insulated overnight hold *is* your heat treatment, and a
  1 400 g block holds >65 °C far longer than a 400 g one. Your larger bags genuinely pasteurise
  better.
- **Geometry — the one that matters most.** Colonisation speed is set by how far mycelium has to
  travel. **Flatten every block to 4–5 cm thick**, spread across the bag, rather than letting it sit
  as a fat lump. At 5 % spawn this is worth more than anything else you can do for free.

### 7.1 Day 0 — tonight

- Confirm the closet sits at **23–25 °C** with a thermometer left in it for a few hours. That is
  your incubator and it is at the right temperature; verify rather than assume.
- Put a **tray or shallow tub** in the closet for the bags to stand in. Seven bags means one may
  leak, and you want that contained and off your clothes.
- Wipe the still-air box down (§3.3) so it is ready.
- Lay out and label seven bags now: **O1 O2 O3 L1 L2 L3 S1**, each with the date and its bran %.
  Label before anything is wet. Colonised oyster, lion's mane and shiitake look identical.
- Check the chamber is running and logging (§7.5 — you will not need it for weeks, but debug it on
  an empty box, never on a live grow).

### 7.2 Day 1 — hydrate and pasteurise

Work **one bag at a time**, start to finish, before opening the next.

#### What the bag is

You want a **filter-patch grow bag** — a heat-tolerant polypropylene bag with a small white
rectangular patch bonded into one face. That patch is a 0.2 µm filter: it lets the block breathe
while blocking spores, and it is the reason the bag can stay shut for eight weeks. Everything below
assumes one.

*No filter patch?* Cut a 2 cm slit high on one face and cover it, generously, with **micropore
tape** (§5). Same job, slightly less reliable. Never use a sealed bag with no gas exchange at all —
the mycelium suffocates in its own CO₂.

#### The steps

1. **Stand the bag in a bucket or a tall pot.** An empty filter bag is floppy and will tip over the
   moment there is hot water in it. The bucket is a stand, nothing more — the substrate never
   touches it.
2. **Roll the top 10 cm of the bag down into a cuff**, like a shirt sleeve. This keeps the sealing
   surface clean and dry while you work. Pellets or water on the fold is what makes a seal leak.
3. **Weigh 600 g of pellets straight into the bag.**
4. **Add the bran dry** — 30 g into O2 and L2, 60 g into O3 and L3, none into O1, L1 or S1 — and
   shake it through the pellets before any water goes in. Dry mixing distributes it far better than
   stirring a wet mass ever will.
5. **Pour the water at a hard rolling boil**, at the volume in §7.0. Not hot-tap, not "boiled a
   minute ago". Pour into the centre, not down the side.
6. **Unroll the cuff and close it immediately.** Fold the top over on itself **three or four times**
   in roughly 3 cm folds — one fold is not a seal — and hold it shut with **two binder clips** (the
   black office kind), one at each end of the fold. Clothes pegs are too weak; they pop open as the
   bag flexes.
7. **Leave the air in.** Do not squeeze the headspace out today. That trapped hot air and steam is
   part of your heat treatment, and this hold is the only heat treatment these bags get.
8. **Stand it upright for 20–30 minutes.** The pellets burst, swell and drink the water. Moving it
   before that just sloshes near-boiling water at your fold.
9. **Then lay it flat and press it out to 4–5 cm thick**, spread across the width of the bag, while
   the mass is still soft and will take a shape. Do this now; in an hour it sets and will not move.
10. **Stack all seven bags together and insulate the pile.** A cooler box, or the pile wrapped in a
    duvet or sleeping bag. Bags share heat — a stack holds temperature far longer than seven bags
    sitting apart, which is why they go under one cover rather than one each. **Hold 8–12 hours;
    overnight is easiest.**

> **Without a stockpot, this single hot-hold is your entire heat treatment.** There is no second
> chance at it, so over-insulate rather than under-insulate. If you later acquire a large pot, §3.1
> describes tyndallisation — three 90-minute steam cycles 24 h apart — which is the proper answer
> for supplemented substrate and would change the odds on O3 and L3 considerably.

### 7.3 Day 2 — inoculate

- **Confirm under 25 °C with a thermometer pushed into the centre of a block.** A bag that feels
  cool through the plastic can still be 40 °C inside, and warm substrate kills mycelium. This is a
  real and common failure, not a formality. If in doubt, wait another few hours.
- **Squeeze test** (§1.5): a few drops, not a stream. The bran bags hold more water than the plain
  ones, so check O2/O3 and L2/L3 individually — **err dry**. Wet plus nitrogen is how you get
  bacteria instead of mushrooms, and those are precisely the bags at risk.
- **Prepare everything before you open anything.** Bags lined up and labelled, clips to hand, spawn
  bags wiped down with 70 % isopropanol and a corner cut off each. Every second a bag sits open is
  the only risk you are actually managing here.
- **Work in the still-air box** if you have one: wipe it, let it settle **10 minutes**, then open
  anything. **If you do not have one, use the pour-and-knead method and a steamed bathroom — both
  in §3.3.** Do not delay a cooled, pasteurised bag to go shopping for a tub; every extra hour
  favours whatever survived the hot-hold.
- **Never put your hands inside a bag.** Open the substrate bag just enough to pour through, pour,
  close it. Hands stay outside, always.
- **73 g of spawn into each of O1–O3 and L1–L3; all 280 g into S1.**
- **Mix obsessively — through the plastic.** Close the bag first, then knead and squeeze until the
  spawn is evenly through the block. At 5 % spawn every grain is a separate colonisation front and
  distribution *is* your colonisation speed, so spend twice as long as feels necessary. Doing it
  sealed costs you nothing and removes the longest exposure in the whole process.
- **Now press most of the air out**, unlike yesterday. Gently flatten the bag against the block so
  the plastic sits on the substrate, then fold and clip as before — three or four folds, two binder
  clips. Less headspace means less condensation and less room for anything airborne to circulate.
  Leave the **filter patch clear and unobstructed**; never fold across it or tape over it.
- Re-flatten to 4–5 cm and lay the bag down **filter patch facing up**, so substrate and condensation
  cannot block it.
- Do S1 **first**, while the box is cleanest. It is the bag you can least afford to lose and the one
  with the longest exposure ahead of it.

Once a bag is closed on day 2, **it stays closed** until it is fully colonised and ready to cut for
fruiting. Every opening is a fresh exposure, and there is nothing inside you can fix by looking.

### 7.4 Incubation — the closet, weeks 1–8

Bags go in the **closet at 23–25 °C, dark**, not in the chamber. This is correct and it is what
§0.2 recommends: the chamber's value is humidity control at fruiting, and incubation just wants
warmth, darkness and to be left alone. Set the dashboard phase to `Incubation` anyway so the log
has a marker.

Expected colonisation at 5 % spawn and 24 °C:

| Bag | Expect | Worry after |
|---|---|---|
| O1 — oyster, 0 % | 30–45 days | 55 days |
| O2 — oyster, 5 % | 28–40 days *if it takes* | see below |
| O3 — oyster, 10 % | 25–38 days *if it takes* | see below |
| L1 — lion's mane, 0 % | 35–55 days | 65 days |
| L2 / L3 — lion's mane, bran | 30–50 days *if they take* | see below |
| **S1 — shiitake** | **8–16 weeks** | 20 weeks |

Two patterns to read here. The bran bags should colonise **faster** than their controls if the
pasteurisation held — that is the whole point of supplementing. And the alder fraction (§1.7) pulls
everything toward the fast end. Slower than these ranges usually means the closet is cooler than you
measured, not that something is wrong.

**Failure shows up early, and it looks like bacteria, not mould.** O3 and L3 are the ones to watch.
In the **first 5–10 days**, before mycelium covers anything:

- A **sour, rancid or sharply sweet smell** through the filter patch — the clearest early signal.
- **Wet, slimy grey or yellow patches** with no fluffy texture.
- Substrate visibly **darker and wetter** than the day you sealed it.

Green *Trichoderma* (§8) is a later and different failure. Either way: **discard the bag sealed,
outdoors, unopened.** Do not try to rescue it, do not open it in the flat, and do not open it near
the other six. Then log it — that is a clean result, not a wasted bag.

**A little condensation on the inside of the bag is normal** — expect it, especially in the first
week. Water actually *pooling* in the bottom means the block went in too wet; there is nothing to be
done about it now, so note it and adjust the hydration next run.

Check every few days. **Do not open a healthy bag to look.**

### 7.5 Capacity — you have more blocks than chamber

§4.1 is the constraint nobody plans for: a 65 L SAMLA holds **3–4 blocks of this size**, and you
will have six, plus the shiitake later. You cannot fruit them all at once.

This solves itself if you let it:

- **A fully colonised block waits happily for weeks** in the closet, and the extra consolidation
  time improves it. There is no rush.
- Colonisation will finish at different times anyway — bran bags first if they take, lion's mane
  last.
- So **fruit in batches of three**, in the order they finish. First three colonised go into the
  chamber; the rest stay in the closet until the chamber is free.
- Keep each batch to **one species where you can**. Oyster and lion's mane can share the chamber
  (both fruit near 18 °C / 88 % under the `OYSTER` config), but harvesting is simpler when a batch
  behaves as one.

### 7.6 Consolidation and pinning

Let a block go **fully white**, then give it **3–5 more days** to consolidate. Impatience here is
the second most common failure after over-hydration.

Then move it to the chamber and set phase to `Primordia`.

- **Oyster (O1–O3)** — target is 13 °C, and §0.2 is blunt that you cannot get there in a heated
  flat. Two workarounds: a **12–24 h stint in the fridge** before it goes in the chamber is a clean
  cold shock, and from mid-October a balcony or unheated room does it for free. Then 93 % RH, light
  on. Cut a slit or X. Pins in 3–7 days.
- **Lion's Mane (L1–L3)** — no cold shock needed; it pins readily at 15–18 °C, which you can hold.
  Cut **one** hole, ~4–5 cm, and let it fruit from that single site rather than everywhere at once.
  Humidity matters more for lion's mane than the temperature drop does.

### 7.7 Fruiting

Set phase to `Fruiting`: **18 °C, 88 % RH, 8–12 h light, maximum fresh air.**

- **Oyster** — fruits double daily. Harvest while cap edges are still **curled down**, usually 5–7
  days from pinning. Flat or upturned caps mean you waited too long.
- **Lion's Mane** — harvest while the spines are **short and the flesh is pure white**. Yellowing or
  browning means over-ripe and bitter. Typically 7–10 days from pinning.

CO₂ is what your build under-serves (§4.2). Long stems with small caps on the oyster, or a
coral-like branching lion's mane instead of a compact ball, both mean **too little fresh air** — not
too little humidity.

Rest each block after a flush, optionally soak it, return to fruiting conditions. Expect 2–3
flushes, then compost the spent block.

### 7.8 The shiitake — a separate, much longer project

S1 is not part of the same grow. Plan it as its own thing:

| Stage | When | What |
|---|---|---|
| Incubation | **Sept → Nov/Dec** | Closet, 24–25 °C, dark. 8–16 weeks. Do not disturb |
| Browning | **+2–4 weeks** | The block turns brown and forms a leathery skin. This is correct and expected, not contamination. Give it light and leave it sealed |
| Fruiting | **Dec → Feb** | 13 °C, 75 % RH — needs the chamber to itself |

At 600 g of pellets and a 20 % spawn rate this is a full-size block, not a trial — if it comes
through the winter it is a real harvest.

**The timing is accidentally ideal.** §0.2 says shiitake at 13 °C is impossible in a heated flat
and recommends fruiting it in the cold half of the Swedish year. Starting today, S1 becomes ready
in **December or January**, when an unheated room or glassed balcony in Gothenburg sits at 8–16 °C.
You get the cooling for free, exactly as §0.2 hoped.

Two things to remember months from now:

- **Change the firmware to `getMushroomConfig(SHIITAKE)`** before fruiting it. Shiitake wants
  13 °C / 75 % RH against oyster's 18 °C / 88 % — genuinely incompatible, so it cannot share the
  chamber with an oyster batch.
- **Shiitake needs a shock to fruit.** Soak the browned block in cold water for 12–24 h, then put
  it into fruiting conditions.

### 7.9 What to record, and what it tells you

Weigh **every harvest** and compute BE per bag (§1.2). The grow log captures chamber conditions
automatically, but it cannot weigh mushrooms — that part is on you, and without it the whole bran
ladder tells you nothing.

Log for each bag: date sealed, date fully colonised, date of first pin, and fresh weight per flush.

Then read the ladder:

- **All three levels colonised clean, 10 % fastest and heaviest** → your pasteurisation is better
  than the literature predicts. Push to 15 % *with* a pressure cooker and stop worrying.
- **0 % and 5 % fine, 10 % failed** → textbook result, and the guide's ceiling (§1.2) is right for
  your kitchen. Run 5 % as standard until you own a *tryckkokare*.
- **Only 0 % survived** → pasteurisation alone cannot carry bran here. Keep the vetekli for the day
  the pressure cooker arrives.
- **Oyster survived where lion's mane did not, at the same bran level** → not a bran result. That is
  oyster's contamination tolerance carrying it, and it says your technique is the variable to work
  on, not the formula.
- **Everything colonised but slowly, with thin yields** → the 5 % spawn rate, exactly as expected.
  The fix next time is more spawn, not more bran — and §2.4 shows how to make your own so it stops
  being the scarce input.

## 8. Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| Long stems, tiny caps | **CO₂ too high** | More fresh air exchange — the #1 chamber-grow problem |
| No pins after full colonisation | No trigger | Cold shock, raise humidity, ensure light, cut the bag |
| Fuzzy white "mould" on pins | **Usually healthy aerial mycelium** | Raise humidity, increase FAE. Rarely a real problem |
| Green mould | *Trichoderma* | Discard sealed, **outdoors**. Review sterile technique |
| **Sour/rancid smell, slimy grey patches, in the first 5–10 days** | **Bacterial bloom** — supplemented substrate that outran its heat treatment | Discard sealed, **outdoors, unopened**. Sterilise, do not pasteurise, next time (§1.2) |
| Sour smell later, no slime | Substrate too wet | Discard; less water next time |
| Colonisation stalls | Too cold / too dry / weak spawn | Check temp is 24 °C; check hydration |
| Dry, cracked cap surfaces | Humidity too low | Check humidifier and RH sensor calibration |
| Mushrooms abort, go yellow/brown | Humidity crash or CO₂ spike | Check the humidity state machine is cycling |
| Tiny first flush | Under-supplemented or under-spawned | Raise spawn rate; supplement and sterilise |

---

## 9. Cost reality check

Rough per-kilo cost of fresh oyster mushrooms, at 75 % BE, one 2 kg block:

| Line | Cost |
|---|---|
| Hardwood pellets (~0.85 kg dry @ 25 kr/kg) | ~21 kr |
| Spawn at 10 % (amortised, bought) | ~30 kr |
| Electricity, water | ~5 kr |
| **Per block (~1.5 kg fresh over all flushes)** | **~56 kr** |
| **Per kg fresh** | **~37 kr/kg** |

Straw by the sack would put substrate at ~3 kr and the total near 25 kr/kg — that gap is the real
price of the pack-size trap in §1.7, and it only closes if you buy 13 kg at a time and drive for
it. Buying hardwood pellets in 30 kg rather than 8 kg narrows most of it without the trip.

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
