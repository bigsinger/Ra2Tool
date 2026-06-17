#pragma once
#include <array>
#include <functional>
#include <span>
#include <string>

struct YGNode;
struct YGConfig;
typedef struct YGNode* YGNodeRef;
typedef struct YGConfig* YGConfigRef;

namespace ra2overlay {
namespace gui {

using ToolId = uint32_t;
static constexpr uint32_t kMaxToolCount = 30;
static constexpr ToolId kInvalidToolId = kMaxToolCount;

using OnToolActive = std::function<void(ToolId, bool)>;
using OnToolReload = std::function<void(ToolId)>;

struct ToolDesc {
  ToolId id;
  std::string desc;
  OnToolActive cb_active;
  OnToolReload cb_reload;
};

class Pannel {
 public:
  Pannel();
  ~Pannel();

  void UpdateTools(std::span<const ToolDesc> tools);
  bool IsMouseInteracting() const;
  bool IsKeyboardInteracting() const;
  // Invoke after window poll events.
  void HandleEvent();
  void Render();

 private:
  enum class GuiId {
    kWindow,
    kTitleBar,
    kIcon,
    kTitle,
    kCheckBoxActive,
    kDesc,
    kButtonUpdate,
  };

  struct NodeDesc {
    GuiId gui_id;
    ToolId tool_id;
  };

  struct Rect {
    float x;
    float y;
    float width;
    float height;
  };

  // Event data.
  // Determine mouse left button click event once.
  uint8_t last_mouse_left_button_state_ = 0;
  bool dragging_ = false;
  float drag_mouse_offset_x_ = 0.0f;
  float drag_mouse_offset_y_ = 0.0f;

  // Render data.
  YGConfigRef yg_config_;
  YGNodeRef yg_root_;
  int pannel_title_height_ = 30;
  int pannel_x_ = 250;
  int pannel_y_ = 150;
  int pannel_width_ = 250;
  // Update by yoga calculation result.
  int pannel_height_ = 0;

  // Tool data.
  std::array<bool, kMaxToolCount> tool_slots_;
  std::array<std::string, kMaxToolCount> tool_desc_;
  std::array<bool, kMaxToolCount> tool_ckbox_active_last_state_;
  std::array<bool, kMaxToolCount> tool_ckbox_active_state_;
  std::array<bool, kMaxToolCount> tool_btn_update_state_;
  std::array<OnToolActive, kMaxToolCount> tool_ckbox_cb_;
  std::array<OnToolReload, kMaxToolCount> tool_btn_cb_;

  static NodeDesc* CreateNodeDesc(YGNodeRef node, NodeDesc&& desc);
  static void DeleteNodeDesc(YGNodeRef node);
  static void FreeContextRecursive(YGNodeRef node);

  void SetupLayout(YGNodeRef root, std::span<const ToolDesc> tools);
  void HandleDragging();
  void HandleUserEvent();
  void DrawGuiWindow(const NodeDesc& /*desc*/, Rect bound);
  void DrawGuiTitleBar(const NodeDesc& /*desc*/, Rect bound);
  void DrawGuiIcon(const NodeDesc& /*desc*/, Rect bound);
  void DrawGuiTitle(const NodeDesc& /*desc*/, Rect bound);
  void DrawGuiCheckBoxActive(const NodeDesc& desc, Rect bound);
  void DrawGuiDesc(const NodeDesc& desc, Rect bound);
  void DrawGuiButtonUpdate(const NodeDesc& desc, Rect bound);
  void DrawGuiWidget(const NodeDesc& desc, Rect bound);
  void DrawGui(YGNodeRef node, float parent_x = 0, float parent_y = 0,
               int depth = 0);
};

}  // namespace gui
}  // namespace ra2overlay
