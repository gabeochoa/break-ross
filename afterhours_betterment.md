# Afterhours UI Framework Improvement Ideas

Ideas for making the afterhours UI framework better or easier to use, based on experience migrating break_ross UI to use the framework.

## Current Work in Progress

**Stash**: `stash@{0}: On main: WIP: UI migration to afterhours - buttons not correctly parenting in container`

**Status**: Migration in progress - encountering layout and parenting issues.

**Current Issues Being Debugged**:

1. **Buttons Not Parenting Correctly in Container**
   - Buttons appear to the right of their parent `buttons_container` instead of inside it
   - `buttons_container` is a child of `shop_container` (which uses `with_absolute_position()`)
   - Buttons use `percent(1.f)` for width and should fill the container
   - Container uses `FlexDirection::Column` but buttons still don't layout correctly
   - Tried: `children()` vs `percent(1.f)` for container height, `with_margin()` vs `with_padding()` for button spacing

2. **Children of Absolutely Positioned Containers**
   - `shop_container` uses `with_absolute_position()` and `with_translate()` for screen positioning
   - Children (`shop_title`, `buttons_container`) use normal layout with `percent()` sizing
   - Children appear to be positioned incorrectly relative to parent
   - Unclear if children of absolutely positioned parents need special configuration

3. **Container Sizing Confusion**
   - When to use `children()` vs `percent(1.f)` vs explicit `pixels()` for container height
   - `buttons_container` with `percent(1.f)` height tries to fill parent, but parent has fixed height
   - `children()` should size to content, but may not work correctly with flex layout
   - Need clearer guidance on sizing strategies for nested containers

4. **Flex Layout Behavior**
   - `buttons_container` uses `FlexDirection::Column` but buttons don't stack vertically as expected
   - Buttons have `percent(1.f)` width but appear outside container bounds
   - Padding/margin on container may be affecting child positioning
   - Need to understand how flex layout interacts with absolutely positioned parents

**Files Modified** (in stash):
- `src/systems/RenderGameUI.h` - Complete rewrite to use afterhours UI
- `src/game.cpp` - UI system registration
- `src/preload.cpp` - Font loading and root UI entity setup
- `src/game_setup.cpp` - Minor changes
- `src/systems/RenderUIDebug.h` - Custom debug overlay (untracked)
- `vendor/afterhours/src/plugins/ui/rendering.h` - Frame-one dump (untracked)
- `vendor/afterhours/src/plugins/ui/components.h` - Fixed duplicate translate bug (untracked)

## API Improvements

### 1. Simplified Include Path
**Issue**: Need to include multiple specific headers or know to include `<afterhours/src/plugins/ui.h>` to get all UI functionality.

**Suggestion**: Consider making the main UI header more discoverable, or provide a single umbrella header that includes everything needed for immediate mode UI.

### 2. ComponentConfig Builder Pattern
**Current**: ComponentConfig uses method chaining which is good, but some common patterns could be simplified.

**Suggestion**: Consider helper functions for common patterns like:
- `text_label(parent, text, position)` for simple text labels
- `centered_button(parent, text, callback)` for common button patterns
- `container(parent, size, padding)` for common container patterns

### 3. Font Handling
**Issue**: Need to specify `UIComponent::DEFAULT_FONT` explicitly for each component, and it's not clear how fonts are registered with the UI system.

**Suggestion**: 
- Make font registration more explicit and documented
- Consider a default font that's automatically used if no font is specified
- Provide helper functions like `with_default_font()` that use the system default

### 4. Positioning Helpers
**Issue**: Using `screen_pct()` for positioning requires manual calculation of percentages. For absolute positioning, need to use `with_absolute_position()` and then calculate positions manually.

**Suggestion**: 
- Provide helper functions like `position_top_left(x, y)`, `position_center()`, `position_bottom_right(x, y)` that handle absolute positioning more intuitively
- Consider a layout system that makes common positioning patterns easier (e.g., "top-left corner with padding")

### 5. Button Disabled State
**Current**: Can use `.with_disabled()` but it's not immediately clear how disabled buttons are styled or if they're automatically styled differently.

**Suggestion**: 
- Document how disabled state affects button appearance
- Consider automatic styling for disabled buttons (e.g., reduced opacity, grayed out)
- Make it clearer that disabled buttons don't trigger click handlers

### 6. Color System
**Issue**: Need to use `afterhours::Color` explicitly, and mixing `Theme::Usage` with `custom_color` can be confusing.

**Suggestion**:
- Provide helper functions for common colors (e.g., `color_white()`, `color_gray()`, `color_rgb(r, g, b)`)
- Better documentation on when to use `Theme::Usage` vs `custom_color`
- Consider a color palette system that's easier to use

### 7. Entity Management with mk()
**Issue**: Need to provide unique indices to `mk()` to avoid entity ID conflicts. This requires manual index management.

**Suggestion**:
- Consider an auto-incrementing index system for common cases
- Provide `mk_next(parent)` that automatically generates unique IDs
- Better error messages when entity ID conflicts occur, with suggestions on how to fix

### 8. Size Calculation Helpers
**Current**: Need to calculate screen percentages manually (e.g., `screen_pct(0.02f)` for 2% of screen).

**Suggestion**:
- Provide helpers like `screen_pct_x(percent)`, `screen_pct_y(percent)` that are more explicit
- Consider pixel-based helpers that automatically scale: `responsive_px(pixels)` that converts to screen percentage based on a reference resolution

### 9. Documentation and Examples
**Issue**: The API is powerful but learning curve is steep. Need to understand many concepts (ComponentConfig, ComponentSize, Padding, Margin, etc.) before being productive.

**Suggestion**:
- More inline documentation with examples
- A "Getting Started" guide with common patterns
- Example code showing typical UI patterns (button, text label, container, etc.)

### 10. Error Messages
**Issue**: When something goes wrong (e.g., entity ID conflict), error messages could be more helpful.

**Suggestion**:
- More descriptive error messages with context
- Suggestions on how to fix common issues
- Better debugging tools (e.g., UI element inspector)

## Code Quality Improvements

### 11. Type Safety
**Suggestion**: Consider stronger typing for common values:
- `FontSize` type instead of `float`
- `Opacity` type (0.0-1.0) instead of `float`
- `Percent` type (0.0-1.0) instead of `float` for percentages

### 12. Const Correctness
**Suggestion**: Some functions could be more const-correct, making it clearer what can be modified.

## Missing Features

### 13. Text Alignment
**Issue**: Text alignment options exist but it's not immediately clear how to use them effectively.

**Suggestion**: Better documentation and examples of text alignment, especially for buttons and labels.

### 14. Layout Debugging
**Issue**: While there's `ToggleUILayoutDebug`, it would be helpful to have more debugging tools.

**Suggestion**:
- Visual overlay showing component bounds
- Component hierarchy viewer
- Performance profiling for UI rendering

### 15. Responsive Design Helpers
**Issue**: Making responsive UIs requires manual calculation of screen percentages.

**Suggestion**:
- Helper functions that make responsive design easier
- Breakpoint system for different screen sizes
- Automatic scaling based on reference resolution

## Critical Usability Issues

### 16. Text Elements Requiring Background Colors
**Issue**: Text elements with `.with_label()` may not render correctly without an explicit background color. The rendering system checks for `HasColor` OR `HasLabel`, but in practice, text elements often need a background color to be visible or to trigger proper layout calculations.

**Problem**: 
- Unclear whether text-only elements need `with_color_usage()` or `with_custom_color()`
- Text elements without backgrounds may not render or may have layout issues
- No clear documentation on when background colors are required vs optional

**Suggestion**:
- **Documentation**: Clearly document that text elements should either:
  - Have a background color (even if transparent) for proper rendering
  - Or be inside a container with a background color
- **Default Behavior**: Consider automatically adding a transparent background to text-only elements if no color is specified
- **Examples**: Provide clear examples showing text elements with and without backgrounds
- **Validation**: Add runtime warnings or errors when text elements are created without backgrounds in contexts where they might not render

### 17. Absolute Positioning and Layout Confusion
**Issue**: Using `.with_absolute_position()` requires understanding how it interacts with parent containers and layout systems. Children of absolutely positioned elements may not layout correctly if they're not also absolutely positioned.

**Problem**:
- Unclear when to use absolute positioning vs normal layout
- Children of absolutely positioned containers may need special handling
- Mixing absolute and relative positioning can lead to invisible or incorrectly positioned elements

**Suggestion**:
- **Documentation**: Clear examples showing:
  - When to use absolute positioning
  - How children of absolutely positioned elements should be configured
  - Common pitfalls and how to avoid them
- **Helper Functions**: Provide helpers like `absolute_text_label()` that handle the common pattern correctly
- **Layout Warnings**: Add validation that warns when mixing absolute and relative positioning in ways that might cause issues

### 18. Font Configuration and Inheritance
**Issue**: Font configuration is not always clear - when do you need `.with_font()`, when does it inherit, and how does font size work with different screen sizes?

**Problem**:
- Font must be explicitly set on each element or inherited from parent
- Font size calculations for responsive design are manual
- Unclear if font needs to be "active" in FontManager for rendering

**Suggestion**:
- **Documentation**: Explain font inheritance and when explicit font setting is required
- **Default Font**: Make it clearer how the default font works and when it's applied
- **Responsive Fonts**: Provide helpers for responsive font sizing (e.g., `responsive_font_size(base_size)`)
- **Font Validation**: Warn when a font is specified that hasn't been loaded into FontManager

### 19. Entity ID Management with mk()
**Issue**: The `mk(parent, index)` pattern requires manual index management, which is error-prone and makes refactoring difficult.

**Problem**:
- Easy to create duplicate indices when adding/removing elements
- Indices must be unique across all children of a parent
- Refactoring requires updating many index values
- No compile-time checking for index conflicts

**Suggestion**:
- **Auto-incrementing**: Provide `mk_next(parent)` that automatically generates unique sequential IDs
- **String-based IDs**: Consider `mk_id(parent, "unique_string_id")` for more stable, readable IDs
- **Compile-time Checking**: If possible, use constexpr or template magic to detect duplicate indices at compile time
- **Better Error Messages**: When index conflicts occur, provide helpful error messages with suggestions

### 20. ComponentConfig Property Dependencies
**Issue**: Some ComponentConfig properties have hidden dependencies or requirements that aren't obvious. For example, text elements might need background colors, or absolute positioning might require specific parent configurations.

**Problem**:
- Property combinations that don't work aren't documented
- No validation of invalid property combinations
- Easy to create elements that don't render without understanding why

**Suggestion**:
- **Validation**: Add runtime validation that checks for common invalid property combinations
- **Documentation**: Document property dependencies and requirements
- **Helper Functions**: Provide helper functions for common valid combinations (e.g., `text_label_with_background()`)
- **Debug Mode**: In debug builds, log warnings when potentially problematic property combinations are used

### 21. Critical: Margins with Absolute Positioning Cause Negative Sizes
**Issue**: When using `.with_absolute_position()` with margins, the autolayout system subtracts margins from the element's width/height, causing negative sizes and invisible elements.

**Problem**:
- Using `with_margin()` with `with_absolute_position()` results in: `calculated_width = desired_width - margin.left - margin.right`
- This causes negative widths/heights (e.g., width of 512px with left margin of 384px = 128px, but if right margin is also set, it becomes negative)
- Elements with negative sizes don't render but there's no warning
- The workaround (using `with_translate()` instead of margins) is not documented and not intuitive

**Example of the bug**:
```cpp
// This creates an element with NEGATIVE width:
imm::div(context, mk(parent, 0),
    ComponentConfig{}
        .with_size(ComponentSize{screen_pct(0.4f), screen_pct(0.6f)})
        .with_margin(Margin{.left = screen_pct(0.3f), .right = screen_pct(0.3f)})
        .with_absolute_position()
);
// Result: width = 512 - 384 - 384 = -256 (NEGATIVE!)
```

**Suggestion**:
- **Fix the calculation**: For absolutely positioned elements, margins should position the element, NOT affect size calculations
- **Validation**: Add runtime checks that warn/error when negative sizes are calculated
- **Documentation**: Clearly document that margins + absolute positioning don't work together as expected
- **Helper**: Provide `with_absolute_position_at(x, y)` that handles positioning correctly without margin confusion
- **Alternative**: Make `with_translate()` the recommended way to position absolute elements, and deprecate using margins with absolute positioning

### 22. Negative Sizes Not Detected or Warned
**Issue**: The UI system allows elements to have negative widths/heights without any warning, causing silent rendering failures.

**Problem**:
- Elements with negative sizes are created and queued for rendering
- They appear in `render_cmds` but don't render
- No validation or warning that sizes are invalid
- Debugging requires manually checking computed sizes in logs

**Suggestion**:
- **Validation**: Add assertions or warnings when `UIComponent` has negative width or height
- **Early Detection**: Validate sizes immediately after autolayout calculation
- **Debug Overlay**: Show elements with invalid sizes in a different color in debug mode
- **Error Messages**: When an element doesn't render, check if size is negative and provide helpful error message

### 23. Position Logging Doesn't Show Translate
**Issue**: When using `with_translate()` for positioning, the logged position shows as (0, 0) because translate is applied via `HasUIModifiers` during rendering, not stored in the base position.

**Problem**:
- Debug logs show `x=0.0, y=0.0` for translated elements
- Makes debugging positioning issues very difficult
- Have to manually check `HasUIModifiers` component to see actual position
- The debug overlay in `RenderUIDebug` doesn't account for translate

**Suggestion**:
- **Logging**: Include translate values in position logging (e.g., "x=0.0+384.0, y=0.0+144.0")
- **Debug Tools**: Debug overlays should show the final rendered position including translate
- **Helper**: Provide `get_final_position(entity)` function that returns position + translate
- **Documentation**: Document that translate affects rendering position but not logged position

### 24. Margin Calculation with Absolute Positioning is Counter-Intuitive
**Issue**: The interaction between margins and absolute positioning is confusing and leads to bugs. Margins are subtracted from size calculations even for absolutely positioned elements.

**Problem**:
- Developers expect margins to position elements, not affect their size
- The current behavior: `final_width = desired_width - margin.left - margin.right`
- This means you can't use margins to position AND have a specific size
- Forces developers to use `with_translate()` instead, which is not obvious

**Suggestion**:
- **Fix**: For absolutely positioned elements, margins should only affect position, not size
- **Documentation**: Clearly explain the difference between margins (for layout) and translate (for absolute positioning)
- **Deprecation**: Consider deprecating margin usage with absolute positioning
- **Migration Guide**: Provide examples showing old (broken) vs new (working) patterns

### 25. Children of Absolutely Positioned Containers Don't Layout Correctly
**Issue**: When a container uses `with_absolute_position()`, its children (using normal layout with `percent()` sizing) may not position correctly relative to the parent container.

**Problem**:
- Parent container is absolutely positioned on screen (e.g., `shop_container` at x=384, y=144)
- Children use normal layout with `percent(1.f)` width to fill parent
- Children appear outside parent bounds (e.g., buttons appear to the right of `buttons_container`)
- Flex layout (`FlexDirection::Column`) doesn't work as expected for children
- Unclear if children need to also be absolutely positioned, or if there's a configuration issue

**Example**:
```cpp
// Parent container - absolutely positioned
auto shop_container = imm::div(context, mk(parent, 2),
    ComponentConfig{}
        .with_size(ComponentSize{pixels(512), pixels(432)})
        .with_absolute_position()
        .with_translate(384, 144)
        .with_flex_direction(FlexDirection::Column));

// Child container - normal layout
auto buttons_container = imm::div(context, mk(shop_container.ent(), 1),
    ComponentConfig{}
        .with_size(ComponentSize{percent(1.f), children()})
        .with_flex_direction(FlexDirection::Column));

// Buttons - children of buttons_container
imm::button(context, mk(buttons_container.ent(), 0),
    ComponentConfig{}
        .with_size(ComponentSize{percent(1.f), pixels(50)}));
// Result: Buttons appear to the RIGHT of buttons_container, not inside it
```

**Suggestion**:
- **Documentation**: Clearly document how children of absolutely positioned containers should be configured
- **Validation**: Add warnings when mixing absolute positioning with normal layout in ways that might fail
- **Examples**: Provide working examples of nested containers where parent is absolutely positioned
- **Fix**: Investigate if autolayout correctly handles children of absolutely positioned parents
- **Helper**: Provide `absolute_container_with_children()` helper that sets up the pattern correctly

