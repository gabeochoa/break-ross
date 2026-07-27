# break-ross → last_mile-style Street View mapper — working notes

Reshaping break-ross into a zen incremental: an autonomous fleet of Street View
cars maps a real road network; you sell the map data for cash and expand until
the whole world is mapped. Reference project: `~/p/last_mile` (TS/React) —
`DESIGN.md` + `src/config.ts` are the design source of truth (port concepts,
not code).

## Build / run
`make` does NOT track the compile-flag change, so a clean build is required:
```
make clean && make run     # TAB opens the shop
```

## Decisions locked
- Theme: Street View mapping; **cash = selling maps to people**.
- Currency: `$` + `k/M/B/T` — NOT yet implemented (still shows "Pixels", integer costs).
- Reveal: the street map lights up (no hidden photo); `RenderPhotoReveal` stays disabled.
- Mechanic: one drive-by maps a street (bricks/damage cut).
- Core loop: region-completion — map a region → bonus → expand → reveal resets, re-map.
  Same NYC map is a placeholder for every region for now.
- All other answers in the 100-question list defaulted to the assistant's "rec".

## Done (commits on `main`, this work)
- fix: cars no longer get stuck at dead ends (dropped shared static trackers; dead-end turn-around)
- chore: stripped debug logging spam
- refactor: one reveal grid, owned by `IsPhotoReveal` (was duplicated with `FogOfWar`)
- feat: Territory — `World unmapped: %` headline, City→Country→Continent→Earth scopes,
  Map Expansion + Buy Out Rival buttons, region-completion bonus (`TerritoryProgress`)
- refactor: cut the dead brick/collision layer (bricks were already inert)
- feat: micrographic reskin (`src/palette.h`): near-black ground, ink roads that light
  accent when mapped, accent cars, ink HUD
- build: afterhours submodule bump + `-DAFTER_HOURS_UI_SINGLE_COLLECTION` + fmt consteval patch

## Next up (recommended order)
1. Economy slice — `$` + `k/M/B/T` formatter; per-street pay + "Better Rates"; passive
   map-data licensing = "Contracts" ($/s); late ×5/×10/×100 multipliers; progressive UI
   (shop fades in on first affordable); $/s + streets/s readouts. Model on `last_mile/src/config.ts`.
2. Visible rivals — draw rival mapper cars/territory (blue, `palette::RIVAL`) behind the
   buyout meter; gentle share growth.
3. Reskin polish — registration marks + 1px frame, riso grain, mono plate font (needs asset).
4. Save/load (autosave JSON) + ending (unmapped → 0%, stats/credits).

## Gotchas
- Smoke-test logging: our `log_info` → block-buffered stdout, raylib → stderr. With little
  stdout output, our logs are lost on SIGKILL — absence of our logs ≠ failure. Look at the window.
- Background GUI runs here are timing-flaky (O(n²) `build_connected_components` on 2376
  segments + window scheduling). Not a code issue.
- Placeholder image `resources/images/photos/test_photo_500x500.png` is synthetic; unused now.
- Region expansion reuses NYC; real multi-city maps would use `scripts/download_nyc_roads.py`.
- `TerritoryProgress` gates the completion bonus per `expand_level` and resets the reveal on
  expand; `IsPhotoReveal` is the single reveal-grid owner, `FogOfWar` keeps only reachability.
