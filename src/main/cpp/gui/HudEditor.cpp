#include "gui/FltkCallback.hpp" // callbackOn
#include "gui/HudEditor.hpp"    // std::function, std::unique_ptr
#include "gui/HudElement.hpp"
#include "gui/HudLayout.hpp"
#include "gui/PlayerIndicatorWindowAppearance.hpp" // PlayerIndicatorWindow::surface, toRectangle
#include "gui/PlayerIndicatorWindowStyle.hpp"      // PlayerIndicatorWindow::style
#include "gui/Rectangle.hpp"                       // phud::Rectangle
#include "gui/TitleBar.hpp"
#include "gui/WidgetFactory.hpp" // createWidget
#include <algorithm> // std::clamp, std::ranges::for_each
#include <format>
#include <gsl/gsl> // gsl::not_null
#include <string>
#include <string_view>
#include <tuple> // std::ignore

#if defined(_MSC_VER) // removal of specific msvc warnings due to FLTK
#  pragma warning(push)
#  pragma warning(disable : 4191 4244 4365 4458 4514 4625 4626 4820 5026 5027)
#endif // _MSC_VER

#include <FL/Enumerations.H> // Fl_Event
#include <FL/Fl.H>           // Fl::event_*
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/fl_draw.H>

#if defined(_MSC_VER) // end of specific msvc warnings removal
#  pragma warning(pop)
#  pragma warning(disable : 26403 26409)
#endif // _MSC_VER

/*
 * Position of the graphical elements on the HUD editor window
 */
// +------------------------------------------------> X
// | ------------  hint
// | |          |  ---------------------------------
// | |          |  |                               |
// | | elements |  |     grid of the indicator     |
// | |   list   |  |                               |
// | |          |  ---------------------------------
// | |          |  [Add] [Remove]          [Default]
// | |          |  description of the element
// | |          |  sample title
// | |          |  -----------
// | |          |  | sample  |
// | |          |  -----------
// | ------------                      [OK] [Cancel]
// V
// Y

namespace piws = PlayerIndicatorWindow::surface;
namespace style = PlayerIndicatorWindow::style;

namespace {
  constexpr auto SPACE {5};
  constexpr auto BUTTON_WIDTH {80};
  constexpr auto BUTTON_HEIGHT {25};
  constexpr auto HINT_HEIGHT {90};
  constexpr auto DESCRIPTION_HEIGHT {120};
  constexpr auto SAMPLE_TITLE_HEIGHT {20};
  constexpr auto LIST_WIDTH {230};
  constexpr auto PREVIEW_ZOOM {2};
  constexpr auto PREVIEW_CELL_WIDTH {piws::cellWidth * PREVIEW_ZOOM};
  constexpr auto PREVIEW_CELL_HEIGHT {piws::cellHeight * PREVIEW_ZOOM};
  constexpr auto PREVIEW_FONT_SIZE {12};
  constexpr auto RESIZE_MARGIN {6}; // pixels near a side of an element where it can be resized

  constexpr phud::Rectangle HINT {.x = LIST_WIDTH + 2 * SPACE,
                                  .y = SPACE,
                                  .w = piws::nbColumns * PREVIEW_CELL_WIDTH,
                                  .h = HINT_HEIGHT};
  constexpr phud::Rectangle PREVIEW {.x = HINT.x,
                                     .y = HINT.y + HINT.h + SPACE,
                                     .w = HINT.w,
                                     .h = piws::nbRows * PREVIEW_CELL_HEIGHT};
  constexpr auto EDIT_BUTTONS_Y {PREVIEW.y + PREVIEW.h + SPACE};
  constexpr phud::Rectangle DESCRIPTION {.x = HINT.x,
                                         .y = EDIT_BUTTONS_Y + BUTTON_HEIGHT + SPACE,
                                         .w = HINT.w,
                                         .h = DESCRIPTION_HEIGHT};
  constexpr phud::Rectangle SAMPLE_TITLE {.x = HINT.x,
                                          .y = DESCRIPTION.y + DESCRIPTION.h + SPACE,
                                          .w = HINT.w,
                                          .h = SAMPLE_TITLE_HEIGHT};
  // at the real size of a PlayerIndicator, with room for its border
  constexpr phud::Rectangle SAMPLE {.x = HINT.x + 1,
                                    .y = SAMPLE_TITLE.y + SAMPLE_TITLE.h + 1,
                                    .w = piws::nbColumns * piws::cellWidth,
                                    .h = piws::nbRows * piws::cellHeight};
  constexpr auto VALIDATION_BUTTONS_Y {SAMPLE.y + SAMPLE.h + 1 + SPACE};
  constexpr auto WINDOW_WIDTH {HINT.x + HINT.w + SPACE};
  constexpr auto WINDOW_HEIGHT {VALIDATION_BUTTONS_Y + BUTTON_HEIGHT + SPACE};
  constexpr phud::Rectangle LIST {
      .x = SPACE, .y = SPACE, .w = LIST_WIDTH, .h = WINDOW_HEIGHT - 2 * SPACE};

  constexpr auto TITLE {"Customize the HUD"};
  constexpr auto HINT_TEXT {
      "Select an element in the list and add it, then drag it to its place in the grid.\n"
      "Drag the left or right side of an element to change its width.\n"
      "In bold: displayed on the HUD. In italic: no data yet, the HUD displays a dash."};
  constexpr auto NO_DATA_TEXT {"\n\nNo data yet: this statistic is not computed."};
  constexpr auto NO_SELECTION_TEXT {"Select an element to see its description."};
  constexpr auto SAMPLE_TITLE_TEXT {"Sample of a player indicator:"};
  constexpr std::string_view SAMPLE_PLAYER_NAME {"name"};
  constexpr std::string_view SAMPLE_STAT {"99"};

  constexpr Fl_Color FREE_CELL_LINE_COLOR {FL_LIGHT2};
  constexpr Fl_Color ITEM_COLOR {FL_LIGHT1};
  constexpr Fl_Color SELECTED_ITEM_COLOR {FL_YELLOW};
  constexpr Fl_Color VALID_TARGET_COLOR {FL_DARK_GREEN};
  constexpr Fl_Color INVALID_TARGET_COLOR {FL_RED};

  /**
   * @returns the rectangle moved by (dx, dy): from coordinates relative to a widget to coordinates
   * relative to its window, as FLTK draws.
   */
  [[nodiscard]] constexpr phud::Rectangle translate(const phud::Rectangle& r, int dx,
                                                    int dy) noexcept {
    return {.x = r.x + dx, .y = r.y + dy, .w = r.w, .h = r.h};
  }

  /**
   * What the user does with the mouse button pressed on the grid.
   */
  enum class [[nodiscard]] Drag : short { NONE, MOVE, RESIZE_LEFT, RESIZE_RIGHT };

  [[nodiscard]] constexpr bool isResizing(Drag drag) noexcept {
    return Drag::RESIZE_LEFT == drag or Drag::RESIZE_RIGHT == drag;
  }

  /**
   * The grid of a PlayerIndicator, where the user selects the elements, moves them, and changes
   * their width by dragging their left or right side.
   * Its callback is called when the selection or the layout changes.
   */
  class [[nodiscard]] HudPreview final : public Fl_Widget {
  private:
    HudLayout& m_layout;
    const HudElement* m_pSelected {nullptr};
    Drag m_drag {Drag::NONE};
    int m_grabbedColumn {0}; // where the element has been grabbed, relative to its first column
    CellPosition m_target {.row = 0, .column = 0};
    int m_targetWidth {1};

    /**
     * @returns where an element is drawn on the grid.
     */
    [[nodiscard]] phud::Rectangle toRectangle(CellPosition position, int width) const {
      return translate(PlayerIndicatorWindow::toRectangle(position, width, PREVIEW_ZOOM), x(), y());
    }

    /**
     * @returns the cell under the mouse.
     */
    [[nodiscard]] CellPosition eventCell() const {
      return {.row = std::clamp((Fl::event_y() - y()) / PREVIEW_CELL_HEIGHT, 0, piws::nbRows - 1),
              .column =
                  std::clamp((Fl::event_x() - x()) / PREVIEW_CELL_WIDTH, 0, piws::nbColumns - 1)};
    }

    /**
     * @returns the limit between two columns which is the closest to the mouse.
     */
    [[nodiscard]] int eventColumnLimit() const {
      return std::clamp((Fl::event_x() - x() + PREVIEW_CELL_WIDTH / 2) / PREVIEW_CELL_WIDTH, 0,
                        piws::nbColumns);
    }

    [[nodiscard]] const HudItem* itemUnderMouse() const {
      const auto* const pElement = m_layout.elementAt(eventCell());
      return (nullptr == pElement) ? nullptr : m_layout.find(*pElement);
    }

    /**
     * @returns what a click at the mouse position would do: near a side of the item, it resizes it.
     */
    [[nodiscard]] Drag dragUnderMouse(const HudItem* pItem) const {
      if (nullptr == pItem) {
        return Drag::NONE;
      }

      const auto [ix, iy, iw, ih] = toRectangle(pItem->position, pItem->width);
      const auto mouseX = Fl::event_x();

      if (ix + RESIZE_MARGIN > mouseX) {
        return Drag::RESIZE_LEFT;
      }

      return (ix + iw - RESIZE_MARGIN <= mouseX) ? Drag::RESIZE_RIGHT : Drag::MOVE;
    }

    /**
     * Shows the user that a side of an element can be dragged.
     */
    void updateCursor(Drag drag) const {
      window()->cursor(isResizing(drag) ? FL_CURSOR_WE : FL_CURSOR_DEFAULT);
    }

    void drawGrid() const {
      fl_rectf(x(), y(), w(), h(), FL_WHITE);
      fl_color(FREE_CELL_LINE_COLOR);

      for (auto column = 1; piws::nbColumns > column; ++column) {
        const auto lineX = x() + column * PREVIEW_CELL_WIDTH;
        fl_line(lineX, y(), lineX, y() + h() - 1);
      }

      for (auto row = 1; piws::nbRows > row; ++row) {
        const auto lineY = y() + row * PREVIEW_CELL_HEIGHT;
        fl_line(x(), lineY, x() + w() - 1, lineY);
      }
    }

    void drawItem(const HudItem& item) const {
      const auto [ix, iy, iw, ih] = toRectangle(item.position, item.width);
      fl_rectf(ix, iy, iw, ih, (item.pElement == m_pSelected) ? SELECTED_ITEM_COLOR : ITEM_COLOR);
      fl_rect(ix, iy, iw, ih, FL_DARK3);
      fl_color(FL_BLACK);
      fl_font(FL_HELVETICA, PREVIEW_FONT_SIZE);
      fl_draw(item.pElement->shortName.data(), ix, iy, iw, ih, FL_ALIGN_CENTER, nullptr, 0);
    }

    void drawTarget() const {
      const auto [tx, ty, tw, th] = toRectangle(m_target, m_targetWidth);
      const auto isValid = m_layout.canPlace(*m_pSelected, m_target, m_targetWidth);
      fl_rect(tx, ty, tw, th, isValid ? VALID_TARGET_COLOR : INVALID_TARGET_COLOR);
      fl_rect(tx + 1, ty + 1, tw - 2, th - 2, isValid ? VALID_TARGET_COLOR : INVALID_TARGET_COLOR);
    }

    void onPush() {
      const auto* const pItem = itemUnderMouse();
      m_pSelected = (nullptr == pItem) ? nullptr : pItem->pElement;
      m_drag = dragUnderMouse(pItem);

      if (nullptr != pItem) {
        m_grabbedColumn = eventCell().column - pItem->position.column;
        m_target = pItem->position;
        m_targetWidth = pItem->width;
      }

      redraw();
      do_callback();
    }

    /**
     * Moves the dragged side to the closest column limit, keeping at least one cell.
     */
    void onResize() {
      const auto& item = *m_layout.find(*m_pSelected);
      const auto start = item.position.column;
      const auto end = start + item.width;

      if (Drag::RESIZE_LEFT == m_drag) {
        const auto newStart = std::min(eventColumnLimit(), end - 1);
        m_target = {.row = item.position.row, .column = newStart};
        m_targetWidth = end - newStart;
      } else {
        m_target = item.position;
        m_targetWidth = std::max(eventColumnLimit(), start + 1) - start;
      }
    }

    void onDrag() {
      if (isResizing(m_drag)) {
        onResize();
      } else {
        const auto cell = eventCell();
        m_target = {.row = cell.row, .column = cell.column - m_grabbedColumn};
      }

      redraw();
    }

    void onRelease() {
      // a refused move or resize leaves the element as it was
      m_layout.resizeElement(*m_pSelected, m_target, m_targetWidth);
      m_drag = Drag::NONE;
      updateCursor(dragUnderMouse(itemUnderMouse()));
      redraw();
      do_callback();
    }

  protected:
    void draw() override {
      drawGrid();
      std::ranges::for_each(m_layout.getItems(), [this](const auto& item) { drawItem(item); });

      if (Drag::NONE != m_drag) {
        drawTarget();
      }

      fl_rect(x(), y(), w(), h(), FL_DARK3);
    }

    int handle(int e) override {
      switch (e) {
        case Fl_Event::FL_ENTER:
        case Fl_Event::FL_MOVE:
          // returning 1 to FL_ENTER is needed to receive FL_MOVE
          updateCursor(dragUnderMouse(itemUnderMouse()));
          return 1;

        case Fl_Event::FL_LEAVE:
          if (Drag::NONE == m_drag) {
            updateCursor(Drag::NONE);
          }
          return 1;

        case Fl_Event::FL_PUSH:
          onPush();
          return 1;

        case Fl_Event::FL_DRAG:
          if (Drag::NONE != m_drag) {
            onDrag();
          }
          return 1;

        case Fl_Event::FL_RELEASE:
          if (Drag::NONE != m_drag) {
            onRelease();
          }
          return 1;

        default:
          return Fl_Widget::handle(e);
      }
    }

  public:
    HudPreview(const phud::Rectangle& r, HudLayout& layout)
      : Fl_Widget(r.x, r.y, r.w, r.h),
        m_layout {layout} {}
    HudPreview(const HudPreview&) = delete;
    HudPreview(HudPreview&&) = delete;
    HudPreview& operator=(const HudPreview&) = delete;
    HudPreview& operator=(HudPreview&&) = delete;
    ~HudPreview() override = default;

    [[nodiscard]] const HudElement* getSelected() const noexcept { return m_pSelected; }

    void select(const HudElement* pElement) {
      m_pSelected = pElement;
      redraw();
    }
  }; // class HudPreview

  /**
   * @returns the sample text displayed for the element: what a PlayerIndicator would display.
   */
  [[nodiscard]] constexpr std::string_view toSampleText(const HudElement& element) noexcept {
    return ("playerName" == element.id) ? SAMPLE_PLAYER_NAME : SAMPLE_STAT;
  }

  /**
   * A PlayerIndicator at its real size, displaying sample values, so that the user sees what the
   * layout looks like on a table.
   */
  class [[nodiscard]] HudSample final : public Fl_Widget {
  private:
    const HudLayout& m_layout;

    void drawItem(const HudItem& item) const {
      const auto [ix, iy, iw, ih] = translate(PlayerIndicatorWindow::toRectangle(item), x(), y());
      fl_draw(std::string(toSampleText(*item.pElement)).c_str(), ix, iy, iw, ih, style::textAlign,
              nullptr, 0);
    }

  protected:
    void draw() override {
      // the free part of the grid is not displayed on the table
      fl_rectf(x(), y(), w(), h(), FL_BACKGROUND_COLOR);
      const auto indicatorWidth = m_layout.getWidth();
      const auto indicatorHeight = m_layout.getHeight();
      fl_rectf(x(), y(), indicatorWidth, indicatorHeight, style::backgroundColor);
      fl_rect(x() - 1, y() - 1, indicatorWidth + 2, indicatorHeight + 2, FL_DARK3);
      fl_push_clip(x(), y(), w(), h());
      fl_color(style::textColor);
      fl_font(style::font, piws::fontSize);
      std::ranges::for_each(m_layout.getItems(), [this](const auto& item) { drawItem(item); });
      fl_pop_clip();
    }

  public:
    HudSample(const phud::Rectangle& r, const HudLayout& layout)
      : Fl_Widget(r.x, r.y, r.w, r.h),
        m_layout {layout} {}
    HudSample(const HudSample&) = delete;
    HudSample(HudSample&&) = delete;
    HudSample& operator=(const HudSample&) = delete;
    HudSample& operator=(HudSample&&) = delete;
    ~HudSample() override = default;
  }; // class HudSample

  /**
   * @returns the text of the element in the list, with the formatting of Fl_Browser.
   */
  [[nodiscard]] std::string toListLine(const HudElement& element, const HudLayout& layout) {
    return std::format("{}{}{}", layout.contains(element) ? "@b" : "", hasData(element) ? "" : "@i",
                       element.name);
  }

  [[nodiscard]] std::string toDescription(const HudElement& element) {
    return std::format("{}\n\n{}{}", element.name, element.description,
                       hasData(element) ? "" : NO_DATA_TEXT);
  }

  /**
   * @param callback called on the HudEditor implementation, see callbackOn
   */
  [[nodiscard]] gsl::not_null<Fl_Button*> newButton(int x, int y, std::string_view label,
                                                    Fl_Callback* callback, void* pTarget) {
    return createWidget<Fl_Button>(
        {.x = x, .y = y, .w = BUTTON_WIDTH, .h = BUTTON_HEIGHT}, label,
        [callback, pTarget](gsl::not_null<Fl_Button*> button) { button->callback(callback, pTarget); });
  }

  constexpr auto TEXT_ALIGN {FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_WRAP};
} // anonymous namespace

struct [[nodiscard]] HudEditor::Implementation final {
  OnValidate m_onValidate;
  HudLayout m_layout {};
  std::unique_ptr<Fl_Double_Window> m_window {
      std::make_unique<Fl_Double_Window>(WINDOW_WIDTH, WINDOW_HEIGHT, TITLE)};
  Fl_Hold_Browser* m_list {nullptr};
  HudPreview* m_preview {nullptr};
  HudSample* m_sample {nullptr};
  Fl_Box* m_description {nullptr};
  Fl_Button* m_addBtn {nullptr};
  Fl_Button* m_removeBtn {nullptr};

  explicit Implementation(OnValidate onValidate)
    : m_onValidate {std::move(onValidate)} {
    m_window->begin(); // make explicit that every allocated memory will be freed by the parent
    buildWidgets();
    m_window->end();
  }

  Implementation(const Implementation&) = delete;
  Implementation(Implementation&&) = delete;
  Implementation& operator=(const Implementation&) = delete;
  Implementation& operator=(Implementation&&) = delete;
  ~Implementation() = default;

  void buildWidgets() {
    m_list = createWidget<Fl_Hold_Browser>(LIST, [this](gsl::not_null<Fl_Hold_Browser*> list) {
      list->when(FL_WHEN_RELEASE_ALWAYS);
      list->callback(callbackOn<&Implementation::onListChanged>, this);
      std::ranges::for_each(getHudElements(),
                            [list](const auto& e) { list->add(std::string(e.name).c_str()); });
    });

    std::ignore = createWidget<Fl_Box>(HINT, HINT_TEXT,
                                       [](gsl::not_null<Fl_Box*> box) { box->align(TEXT_ALIGN); });

    m_preview = new HudPreview(PREVIEW, m_layout);
    m_preview->callback(callbackOn<&Implementation::refresh>, this);

    m_description = createWidget<Fl_Box>(
        DESCRIPTION, [](gsl::not_null<Fl_Box*> box) { box->align(TEXT_ALIGN); });

    std::ignore = createWidget<Fl_Box>(SAMPLE_TITLE, SAMPLE_TITLE_TEXT, [](gsl::not_null<Fl_Box*> box) {
      box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    });
    m_sample = new HudSample(SAMPLE, m_layout);

    m_addBtn = newButton(HINT.x, EDIT_BUTTONS_Y, "Add", callbackOn<&Implementation::onAdd>, this);
    m_removeBtn = newButton(HINT.x + BUTTON_WIDTH + SPACE, EDIT_BUTTONS_Y, "Remove",
                            callbackOn<&Implementation::onRemove>, this);
    std::ignore = newButton(HINT.x + HINT.w - BUTTON_WIDTH, EDIT_BUTTONS_Y, "Default",
                            callbackOn<&Implementation::onDefault>, this);
    std::ignore = newButton(HINT.x + HINT.w - 2 * BUTTON_WIDTH - SPACE, VALIDATION_BUTTONS_Y, "OK",
                            callbackOn<&Implementation::onOk>, this);
    std::ignore = newButton(HINT.x + HINT.w - BUTTON_WIDTH, VALIDATION_BUTTONS_Y, "Cancel",
                            callbackOn<&Implementation::onCancel>, this);
  }

  void refreshList() const {
    const auto elements = getHudElements();

    for (auto line = 1; m_list->size() >= line; ++line) {
      const auto& element = elements[static_cast<std::size_t>(line) - 1];
      m_list->text(line, toListLine(element, m_layout).c_str());
    }
  }

  void selectInList(const HudElement* pElement) const {
    m_list->deselect();

    if (nullptr != pElement) {
      const auto index = pElement - getHudElements().data();
      m_list->select(gsl::narrow<int>(index) + 1);
    }
  }

  void refreshSelection() const {
    const auto* const pSelected = m_preview->getSelected();

    if (nullptr == pSelected) {
      m_description->copy_label(NO_SELECTION_TEXT);
      m_addBtn->deactivate();
      m_removeBtn->deactivate();
      return;
    }

    const auto isDisplayed = m_layout.contains(*pSelected);
    m_description->copy_label(toDescription(*pSelected).c_str());
    (!isDisplayed and m_layout.findFreePlace(*pSelected).has_value()) ? m_addBtn->activate()
                                                                      : m_addBtn->deactivate();
    isDisplayed ? m_removeBtn->activate() : m_removeBtn->deactivate();
  }

  void refresh() const {
    refreshList();
    selectInList(m_preview->getSelected());
    refreshSelection();
    m_preview->redraw();
    m_sample->redraw();
    m_window->redraw();
  }

  void addSelected() {
    if (const auto* const pSelected = m_preview->getSelected(); nullptr != pSelected) {
      m_layout.addElement(*pSelected);
    }
  }

  void onListChanged() {
    const auto line = m_list->value();
    m_preview->select((0 == line) ? nullptr
                                  : &getHudElements()[static_cast<std::size_t>(line) - 1]);

    if (0 != Fl::event_clicks()) { // double click
      addSelected();
    }

    refresh();
  }

  void onAdd() {
    addSelected();
    refresh();
  }

  void onRemove() {
    if (const auto* const pSelected = m_preview->getSelected(); nullptr != pSelected) {
      m_layout.removeElement(*pSelected);
    }

    refresh();
  }

  void onDefault() {
    m_layout = HudLayout::defaultLayout();
    refresh();
  }

  void onOk() const {
    m_window->hide();
    m_onValidate(m_layout);
  }

  void onCancel() const { m_window->hide(); }
}; // struct HudEditor::Implementation

HudEditor::HudEditor(OnValidate onValidate)
  : m_pImpl {std::make_unique<Implementation>(std::move(onValidate))} {}

HudEditor::~HudEditor() = default;

void HudEditor::open(const HudLayout& layout) {
  if (!m_pImpl->m_window->shown()) {
    m_pImpl->m_layout = layout;
    m_pImpl->m_preview->select(nullptr);
    m_pImpl->refresh();
  }

  m_pImpl->m_window->show();
  matchTitleBarToWindow(*m_pImpl->m_window);
}
