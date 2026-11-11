#include "gui/HudEditor.hpp" // std::function, std::unique_ptr
#include "gui/HudElement.hpp"
#include "gui/HudLayout.hpp"
#include "gui/PlayerIndicatorWindowAppearance.hpp" // PlayerIndicatorWindow::surface
#include "gui/Rectangle.hpp"                       // phud::Rectangle
#include "gui/TitleBar.hpp"
#include <algorithm>                               // std::clamp, std::ranges::for_each
#include <format>
#include <gsl/gsl> // gsl::not_null

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
// | |          |
// | ------------                      [OK] [Cancel]
// V
// Y

namespace piws = PlayerIndicatorWindow::surface;

namespace {
  constexpr auto SPACE {5};
  constexpr auto BUTTON_WIDTH {80};
  constexpr auto BUTTON_HEIGHT {25};
  constexpr auto HINT_HEIGHT {60};
  constexpr auto DESCRIPTION_HEIGHT {120};
  constexpr auto LIST_WIDTH {230};
  constexpr auto PREVIEW_ZOOM {2};
  constexpr auto PREVIEW_CELL_WIDTH {piws::cellWidth * PREVIEW_ZOOM};
  constexpr auto PREVIEW_CELL_HEIGHT {piws::cellHeight * PREVIEW_ZOOM};
  constexpr auto PREVIEW_FONT_SIZE {12};

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
  constexpr auto VALIDATION_BUTTONS_Y {DESCRIPTION.y + DESCRIPTION.h + SPACE};
  constexpr auto WINDOW_WIDTH {HINT.x + HINT.w + SPACE};
  constexpr auto WINDOW_HEIGHT {VALIDATION_BUTTONS_Y + BUTTON_HEIGHT + SPACE};
  constexpr phud::Rectangle LIST {
      .x = SPACE, .y = SPACE, .w = LIST_WIDTH, .h = WINDOW_HEIGHT - 2 * SPACE};

  constexpr auto TITLE {"Customize the HUD"};
  constexpr auto HINT_TEXT {
      "Select an element in the list and add it, then drag it to its place in the grid.\n"
      "In bold: displayed on the HUD. In italic: no data yet, the HUD displays a dash."};
  constexpr auto NO_DATA_TEXT {"\n\nNo data yet: this statistic is not computed."};
  constexpr auto NO_SELECTION_TEXT {"Select an element to see its description."};

  constexpr Fl_Color FREE_CELL_LINE_COLOR {FL_LIGHT2};
  constexpr Fl_Color ITEM_COLOR {FL_LIGHT1};
  constexpr Fl_Color SELECTED_ITEM_COLOR {FL_YELLOW};
  constexpr Fl_Color VALID_TARGET_COLOR {FL_DARK_GREEN};
  constexpr Fl_Color INVALID_TARGET_COLOR {FL_RED};

  [[nodiscard]] constexpr phud::Rectangle toRectangle(int x, int y, const HudElement& element,
                                                      CellPosition position) noexcept {
    return {.x = x + position.column * PREVIEW_CELL_WIDTH,
            .y = y + position.row * PREVIEW_CELL_HEIGHT,
            .w = element.widthInCells * PREVIEW_CELL_WIDTH,
            .h = PREVIEW_CELL_HEIGHT};
  }

  /**
   * The grid of a PlayerIndicator, where the user selects the elements and moves them.
   * Its callback is called when the selection or the layout changes.
   */
  class [[nodiscard]] HudPreview final : public Fl_Widget {
  private:
    HudLayout& m_layout;
    const HudElement* m_pSelected {nullptr};
    bool m_isDragging {false};
    int m_grabbedColumn {0}; // where the element has been grabbed, relative to its first column
    CellPosition m_target {.row = 0, .column = 0};

    /**
     * @returns the cell under the mouse.
     */
    [[nodiscard]] CellPosition eventCell() const {
      return {
          .row = std::clamp((Fl::event_y() - y()) / PREVIEW_CELL_HEIGHT, 0, piws::nbRows - 1),
          .column =
              std::clamp((Fl::event_x() - x()) / PREVIEW_CELL_WIDTH, 0, piws::nbColumns - 1)};
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
      const auto [ix, iy, iw, ih] = toRectangle(x(), y(), *item.pElement, item.position);
      fl_rectf(ix, iy, iw, ih, (item.pElement == m_pSelected) ? SELECTED_ITEM_COLOR : ITEM_COLOR);
      fl_rect(ix, iy, iw, ih, FL_DARK3);
      fl_color(FL_BLACK);
      fl_font(FL_HELVETICA, PREVIEW_FONT_SIZE);
      fl_draw(item.pElement->shortName.data(), ix, iy, iw, ih, FL_ALIGN_CENTER, nullptr, 0);
    }

    void drawTarget() const {
      const auto [tx, ty, tw, th] = toRectangle(x(), y(), *m_pSelected, m_target);
      const auto isValid = m_layout.canPlace(*m_pSelected, m_target);
      fl_rect(tx, ty, tw, th, isValid ? VALID_TARGET_COLOR : INVALID_TARGET_COLOR);
      fl_rect(tx + 1, ty + 1, tw - 2, th - 2,
              isValid ? VALID_TARGET_COLOR : INVALID_TARGET_COLOR);
    }

    void onPush() {
      const auto cell = eventCell();
      m_pSelected = m_layout.elementAt(cell);
      m_isDragging = nullptr != m_pSelected;

      if (m_isDragging) {
        const auto* const pItem = m_layout.find(*m_pSelected);
        m_grabbedColumn = cell.column - pItem->position.column;
        m_target = pItem->position;
      }

      redraw();
      do_callback();
    }

    void onDrag() {
      const auto cell = eventCell();
      m_target = {.row = cell.row, .column = cell.column - m_grabbedColumn};
      redraw();
    }

    void onRelease() {
      m_isDragging = false;
      // a refused move leaves the element at its previous place
      m_layout.moveElement(*m_pSelected, m_target);
      redraw();
      do_callback();
    }

  protected:
    void draw() override {
      drawGrid();
      std::ranges::for_each(m_layout.getItems(), [this](const auto& item) { drawItem(item); });

      if (m_isDragging) {
        drawTarget();
      }

      fl_rect(x(), y(), w(), h(), FL_DARK3);
    }

    int handle(int e) override {
      switch (e) {
        case Fl_Event::FL_PUSH:
          onPush();
          return 1;

        case Fl_Event::FL_DRAG:
          if (m_isDragging) {
            onDrag();
          }
          return 1;

        case Fl_Event::FL_RELEASE:
          if (m_isDragging) {
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

    void select(const HudElement* pElement) noexcept {
      m_pSelected = pElement;
      redraw();
    }
  }; // class HudPreview

  /**
   * @returns the text of the element in the list, with the formatting of Fl_Browser.
   */
  [[nodiscard]] std::string toListLine(const HudElement& element, const HudLayout& layout) {
    return std::format("{}{}{}", layout.contains(element) ? "@b" : "",
                       hasData(element) ? "" : "@i", element.name);
  }

  [[nodiscard]] std::string toDescription(const HudElement& element) {
    return std::format("{}\n\n{}{}", element.name, element.description,
                       hasData(element) ? "" : NO_DATA_TEXT);
  }

  [[nodiscard]] gsl::not_null<Fl_Button*> newButton(int x, int y, const char* label) {
    return new Fl_Button(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, label);
  }
} // anonymous namespace

struct [[nodiscard]] HudEditor::Implementation final {
  OnValidate m_onValidate;
  HudLayout m_layout {};
  std::unique_ptr<Fl_Double_Window> m_window {
      std::make_unique<Fl_Double_Window>(WINDOW_WIDTH, WINDOW_HEIGHT, TITLE)};
  Fl_Hold_Browser* m_list {nullptr};
  HudPreview* m_preview {nullptr};
  Fl_Box* m_description {nullptr};
  Fl_Button* m_addBtn {nullptr};
  Fl_Button* m_removeBtn {nullptr};

  explicit Implementation(OnValidate onValidate)
    : m_onValidate {std::move(onValidate)} {
    buildWidgets();
    m_window->end();
  }

  void buildWidgets() {
    const auto [lx, ly, lw, lh] = LIST;
    m_list = new Fl_Hold_Browser(lx, ly, lw, lh);
    m_list->when(FL_WHEN_RELEASE_ALWAYS);
    m_list->callback(onListChanged, this);
    std::ranges::for_each(getHudElements(), [this](const auto& e) {
      m_list->add(std::string(e.name).c_str());
    });

    const auto [hx, hy, hw, hh] = HINT;
    auto* const pHint = new Fl_Box(hx, hy, hw, hh, HINT_TEXT);
    pHint->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);

    m_preview = new HudPreview(PREVIEW, m_layout);
    m_preview->callback(onPreviewChanged, this);

    const auto [dx, dy, dw, dh] = DESCRIPTION;
    m_description = new Fl_Box(dx, dy, dw, dh);
    m_description->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);

    m_addBtn = newButton(HINT.x, EDIT_BUTTONS_Y, "Add");
    m_addBtn->callback(onAdd, this);
    m_removeBtn = newButton(HINT.x + BUTTON_WIDTH + SPACE, EDIT_BUTTONS_Y, "Remove");
    m_removeBtn->callback(onRemove, this);
    newButton(HINT.x + HINT.w - BUTTON_WIDTH, EDIT_BUTTONS_Y, "Default")
        ->callback(onDefault, this);
    newButton(HINT.x + HINT.w - 2 * BUTTON_WIDTH - SPACE, VALIDATION_BUTTONS_Y, "OK")
        ->callback(onOk, this);
    newButton(HINT.x + HINT.w - BUTTON_WIDTH, VALIDATION_BUTTONS_Y, "Cancel")
        ->callback(onCancel, this);
  }

  void refreshList() const {
    const auto elements = getHudElements();

    for (auto line = 1; m_list->size() >= line; ++line) {
      const auto& element = elements[static_cast<std::size_t>(line - 1)];
      m_list->text(line, toListLine(element, m_layout).c_str());
    }
  }

  void selectInList(const HudElement* pElement) const {
    m_list->deselect();

    if (nullptr != pElement) {
      const auto index = pElement - getHudElements().data();
      m_list->select(static_cast<int>(index) + 1);
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
    m_window->redraw();
  }

  void addSelected() {
    if (const auto* const pSelected = m_preview->getSelected(); nullptr != pSelected) {
      m_layout.addElement(*pSelected);
    }
  }

  static void onListChanged(Fl_Widget*, void* hiddenSelf) {
    auto& self = *static_cast<Implementation*>(hiddenSelf);
    const auto line = self.m_list->value();
    self.m_preview->select((0 == line) ? nullptr
                                       : &getHudElements()[static_cast<std::size_t>(line - 1)]);

    if (0 != Fl::event_clicks()) { // double click
      self.addSelected();
    }

    self.refresh();
  }

  static void onPreviewChanged(Fl_Widget*, void* hiddenSelf) {
    static_cast<Implementation*>(hiddenSelf)->refresh();
  }

  static void onAdd(Fl_Widget*, void* hiddenSelf) {
    auto& self = *static_cast<Implementation*>(hiddenSelf);
    self.addSelected();
    self.refresh();
  }

  static void onRemove(Fl_Widget*, void* hiddenSelf) {
    auto& self = *static_cast<Implementation*>(hiddenSelf);

    if (const auto* const pSelected = self.m_preview->getSelected(); nullptr != pSelected) {
      self.m_layout.removeElement(*pSelected);
    }

    self.refresh();
  }

  static void onDefault(Fl_Widget*, void* hiddenSelf) {
    auto& self = *static_cast<Implementation*>(hiddenSelf);
    self.m_layout = HudLayout::defaultLayout();
    self.refresh();
  }

  static void onOk(Fl_Widget*, void* hiddenSelf) {
    auto& self = *static_cast<Implementation*>(hiddenSelf);
    self.m_window->hide();
    self.m_onValidate(self.m_layout);
  }

  static void onCancel(Fl_Widget*, void* hiddenSelf) {
    static_cast<Implementation*>(hiddenSelf)->m_window->hide();
  }
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
  matchTitleBarToBackground(*m_pImpl->m_window);
}
