#pragma once

#include <functional> // std::function
#include <memory>     // std::unique_ptr

// forward declaration
class HudLayout;

/**
 * The window where the user chooses what the PlayerIndicators display, and where.
 */
class [[nodiscard]] HudEditor final {
public:
  struct Implementation;
  using OnValidate = std::function<void(const HudLayout&)>;

private:
  std::unique_ptr<Implementation> m_pImpl;

public:
  /**
   * @param onValidate called when the user accepts the changes
   */
  explicit HudEditor(OnValidate onValidate);
  HudEditor(const HudEditor&) = delete;
  HudEditor(HudEditor&&) = delete;
  HudEditor& operator=(const HudEditor&) = delete;
  HudEditor& operator=(HudEditor&&) = delete;
  ~HudEditor();

  /**
   * Shows the window with the given layout, or raises the window if it is already opened: the
   * changes in progress are kept.
   */
  void open(const HudLayout& layout);
}; // class HudEditor
