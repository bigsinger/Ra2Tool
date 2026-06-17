#include "pannel.h"

#include <algorithm>
#include <ranges>

#include "logging.h"
#include "raylib.h"
#define INCLUDE_RAYGUI_AFTER_RAYLIB // prevent linter move raygui.h up
#include "raygui.h"
#include "yoga/Yoga.h"

namespace ra2overlay {
namespace gui {

namespace {
// Avoid exporting raylib Rectangle type to header.
template <class Rect>
Rectangle ToRaylibRect(Rect rect) {
  return Rectangle{rect.x, rect.y, rect.width, rect.height};
}
}  // namespace

Pannel::Pannel() {
  yg_config_ = YGConfigNew();
  YGConfigSetUseWebDefaults(yg_config_, false);
  yg_root_ = YGNodeNewWithConfig(yg_config_);
  UpdateTools({});
}

Pannel::~Pannel() {
  FreeContextRecursive(yg_root_);
  YGNodeFreeRecursive(yg_root_);
  YGConfigFree(yg_config_);
}

void Pannel::UpdateTools(std::span<const ToolDesc> tools) {
  SetupLayout(yg_root_, tools);

  std::ranges::fill(tool_slots_, false);
  std::ranges::fill(tool_desc_, "");
  std::ranges::fill(tool_ckbox_cb_, nullptr);
  std::ranges::fill(tool_btn_cb_, nullptr);
  for (const ToolDesc& tool : tools) {
    tool_slots_[tool.id] = true;
    tool_desc_[tool.id] = tool.desc;
    DCHECK_NOTNULL(tool.cb_active);
    tool_ckbox_cb_[tool.id] = tool.cb_active;
    DCHECK_NOTNULL(tool.cb_reload);
    tool_btn_cb_[tool.id] = tool.cb_reload;
  }
  std::ranges::fill(tool_ckbox_active_last_state_, false);
  std::ranges::fill(tool_ckbox_active_state_, false);
  std::ranges::fill(tool_btn_update_state_, false);
}

bool Pannel::IsMouseInteracting() const {
  Rectangle pannel_title_rect{
      static_cast<float>(pannel_x_),
      static_cast<float>(pannel_y_),
      static_cast<float>(pannel_width_),
      static_cast<float>(pannel_height_),
  };
  Vector2 mouse_point = GetMousePosition();
  return CheckCollisionPointRec(mouse_point, pannel_title_rect);
}

bool Pannel::IsKeyboardInteracting() const {
  return IsMouseInteracting();
}

void Pannel::HandleEvent() {
  HandleDragging();
  HandleUserEvent();
}

void Pannel::Render() {
  YGNodeCalculateLayout(yg_root_, YGUndefined, YGUndefined, YGDirectionLTR);

  ClearBackground(DARKGRAY);
  DrawGui(yg_root_, static_cast<float>(pannel_x_), static_cast<float>(pannel_y_));
}

Pannel::NodeDesc* Pannel::CreateNodeDesc(YGNodeRef node, NodeDesc&& desc) {
  void* ptr = YGNodeGetContext(node);
  if (ptr != nullptr) {
    delete ptr;
  }
  auto desc_ptr = new NodeDesc(std::move(desc));
  YGNodeSetContext(node, desc_ptr);
  return desc_ptr;
}

void Pannel::DeleteNodeDesc(YGNodeRef node) {
  void* ptr = YGNodeGetContext(node);
  if (ptr != nullptr) {
    delete ptr;
  }
  YGNodeSetContext(node, nullptr);
}

void Pannel::FreeContextRecursive(YGNodeRef node) {
  DeleteNodeDesc(node);
  const int child_count = YGNodeGetChildCount(node);
  for (int i = 0; i < child_count; ++i) {
    FreeContextRecursive(YGNodeGetChild(node, i));
  }
}

// https://www.yogalayout.dev/
// <Layout config={{useWebDefaults: false}}>
//   <Node style={{width: 250}}>
//       <Node style={{flexDirection: 'row', alignItems: 'center', height:30}}>
//         <Node style={{width: 20, height: 20, margin: 10}}/>
//         <Node style={{width: 90, height: 20}}/>
//       </Node>
//       <Node style={{flexDirection: 'row', padding: 8, margin: 5}}>
//         <Node style={{width: 20, height: 20, marginInline: 15}} />
//         <Node style={{width: 100, height: 20}} />
//         <Node style={{width: 20, height: 20, marginInline: 15}} />
//       </Node>
//       <Node style={{flexDirection: 'row', padding: 8, margin: 5}}>
//         <Node style={{width: 20, height: 20, marginInline: 15}} />
//         <Node style={{width: 100, height: 20}} />
//         <Node style={{width: 20, height: 20, marginInline: 15}} />
//       </Node>
//   </Node>
// </Layout>
void Pannel::SetupLayout(YGNodeRef root, std::span<const ToolDesc> tools) {
  YGNodeRemoveAllChildren(root);
  YGNodeStyleSetWidth(root, static_cast<float>(pannel_width_));
  CreateNodeDesc(root,
                 NodeDesc{.gui_id = GuiId::kWindow, .tool_id = kInvalidToolId});
  {
    YGNodeRef titlebar = YGNodeNew();
    YGNodeStyleSetFlexDirection(titlebar, YGFlexDirectionRow);
    YGNodeStyleSetAlignItems(titlebar, YGAlignCenter);
    YGNodeStyleSetHeight(titlebar, static_cast<float>(pannel_title_height_));
    YGNodeInsertChild(root, titlebar, 0);
    CreateNodeDesc(titlebar, NodeDesc{.gui_id = GuiId::kTitleBar,
                                      .tool_id = kInvalidToolId});
    {
      YGNodeRef icon = YGNodeNew();
      YGNodeStyleSetWidth(icon, 20);
      YGNodeStyleSetHeight(icon, 20);
      YGNodeStyleSetMargin(icon, YGEdgeAll, 10);
      YGNodeInsertChild(titlebar, icon, 0);
      CreateNodeDesc(
          icon, NodeDesc{.gui_id = GuiId::kIcon, .tool_id = kInvalidToolId});

      YGNodeRef title = YGNodeNew();
      YGNodeStyleSetWidth(title, 90);
      YGNodeStyleSetHeight(title, 20);
      YGNodeInsertChild(titlebar, title, 1);
      CreateNodeDesc(
          title, NodeDesc{.gui_id = GuiId::kTitle, .tool_id = kInvalidToolId});
    }
  }
  for (auto [idx, tool] : std::views::enumerate(tools)) {
    YGNodeRef background = YGNodeNew();
    YGNodeStyleSetFlexDirection(background, YGFlexDirectionRow);
    YGNodeStyleSetPadding(background, YGEdgeAll, 8);
    YGNodeStyleSetMargin(background, YGEdgeAll, 5);
    YGNodeInsertChild(root, background, 1 + idx);
    {
      YGNodeRef check_box = YGNodeNew();
      YGNodeStyleSetWidth(check_box, 20);
      YGNodeStyleSetHeight(check_box, 20);
      YGNodeStyleSetMargin(check_box, YGEdgeHorizontal, 15);
      YGNodeInsertChild(background, check_box, 0);
      CreateNodeDesc(check_box, NodeDesc{.gui_id = GuiId::kCheckBoxActive,
                                         .tool_id = tool.id});

      YGNodeRef desc = YGNodeNew();
      YGNodeStyleSetWidth(desc, 100);
      YGNodeStyleSetHeight(desc, 20);
      YGNodeInsertChild(background, desc, 1);
      CreateNodeDesc(desc,
                     NodeDesc{.gui_id = GuiId::kDesc, .tool_id = tool.id});

      YGNodeRef update_button = YGNodeNew();
      YGNodeStyleSetWidth(update_button, 20);
      YGNodeStyleSetHeight(update_button, 20);
      YGNodeStyleSetMargin(update_button, YGEdgeHorizontal, 15);
      YGNodeInsertChild(background, update_button, 2);
      CreateNodeDesc(update_button, NodeDesc{.gui_id = GuiId::kButtonUpdate,
                                             .tool_id = tool.id});
    }
  }
}

void Pannel::DrawGuiWindow(const NodeDesc& /*desc*/, Rect bound) {
  DrawRectangleRec(ToRaylibRect(bound), RAYWHITE);
  pannel_height_ = static_cast<int>(bound.height);
}

void Pannel::DrawGuiTitleBar(const NodeDesc& /*desc*/, Rect bound) {
  DrawRectangleRec(ToRaylibRect(bound), GRAY);
}

void Pannel::DrawGuiIcon(const NodeDesc& /*desc*/, Rect bound) {
  // TODO
  DrawRectangleRec(ToRaylibRect(bound), ORANGE);
}

void Pannel::DrawGuiTitle(const NodeDesc& /*desc*/, Rect bound) {
  DrawTextEx(GuiGetFont(), "SCARLET", Vector2{bound.x, bound.y}, bound.height,
             1.2f, RED);
}

void Pannel::DrawGuiCheckBoxActive(const NodeDesc& desc, Rect bound) {
  ToolId tool_id = desc.tool_id;
  DCHECK_LT(tool_id, tool_ckbox_active_state_.size());
  DCHECK(tool_slots_[tool_id]);
  bool checked = tool_ckbox_active_state_[tool_id];
  GuiCheckBox(ToRaylibRect(bound), "", &checked);
  tool_ckbox_active_state_[tool_id] = checked;
}

void Pannel::DrawGuiDesc(const NodeDesc& desc, Rect bound) {
  ToolId tool_id = desc.tool_id;
  DCHECK_LT(tool_id, tool_desc_.size());
  DCHECK(tool_slots_[tool_id]);
  DrawTextEx(GuiGetFont(), tool_desc_[tool_id].c_str(),
             Vector2{bound.x, bound.y}, bound.height, 1.2f, BLACK);
}

void Pannel::DrawGuiButtonUpdate(const NodeDesc& desc, Rect bound) {
  int result = GuiButton(ToRaylibRect(bound), "");
  ToolId tool_id = desc.tool_id;
  DCHECK_LT(tool_id, tool_btn_update_state_.size());
  DCHECK(tool_slots_[tool_id]);
  bool pressed = result == 1;
  tool_btn_update_state_[tool_id] = pressed;
}

#define CASE_DRAW(xxx) case GuiId::k##xxx: DrawGui##xxx(desc, bound); break
void Pannel::DrawGuiWidget(const NodeDesc& desc, Rect bound) {
  switch (desc.gui_id) {
    CASE_DRAW(Window);
    CASE_DRAW(TitleBar);
    CASE_DRAW(Icon);
    CASE_DRAW(Title);
    CASE_DRAW(CheckBoxActive);
    CASE_DRAW(Desc);
    CASE_DRAW(ButtonUpdate);
    default:
      UNREACHABLE();
  }
}
#undef CASE_DRAW

void Pannel::DrawGui(YGNodeRef node, float parent_x, float parent_y,
                     int depth) {
  // Relative layout.
  float rel_left = YGNodeLayoutGetLeft(node);
  float rel_top = YGNodeLayoutGetTop(node);
  float width = YGNodeLayoutGetWidth(node);
  float height = YGNodeLayoutGetHeight(node);
  // Absolute layout.
  float abs_left = parent_x + rel_left;
  float abs_top = parent_y + rel_top;
  // const std::string indent(depth * 2, ' ');
  // DLOG_F(INFO, "{}Level {}: ({:.1f}, {:.1f} {:.1f} x {:.1f})", indent,
  //        depth + 1, abs_left, abs_top, width, height);
  Color color{0, (uint8_t)(depth * 30), 0, 255};
  auto desc = reinterpret_cast<NodeDesc*>(YGNodeGetContext(node));
  if (desc != nullptr) {
    DrawGuiWidget(*desc, Rect{abs_left, abs_top, width, height});
  }
  // Recursively draw children.
  const int child_count = YGNodeGetChildCount(node);
  for (int i = 0; i < child_count; ++i) {
    DrawGui(YGNodeGetChild(node, i), abs_left, abs_top, depth + 1);
  }
}

void Pannel::HandleDragging() {
  uint8_t mouse_left_button_down =
      static_cast<uint8_t>(IsMouseButtonDown(MOUSE_LEFT_BUTTON));
  // Detect click event.
  uint8_t mouse_click =
      (last_mouse_left_button_state_ ^ mouse_left_button_down) &
      mouse_left_button_down;
  last_mouse_left_button_state_ = mouse_left_button_down;
  Vector2 mouse_point = GetMousePosition();
  if (mouse_click != 0) {
    Rectangle pannel_title_rect{
        static_cast<float>(pannel_x_),
        static_cast<float>(pannel_y_),
        static_cast<float>(pannel_width_),
        static_cast<float>(pannel_title_height_),
    };
    if (CheckCollisionPointRec(mouse_point, pannel_title_rect)) {
      dragging_ = true;
      drag_mouse_offset_x_ = mouse_point.x - static_cast<float>(pannel_x_);
      drag_mouse_offset_y_ = mouse_point.y - static_cast<float>(pannel_y_);
    }
  }
  if (dragging_ && mouse_left_button_down != 0) {
    pannel_x_ = static_cast<int>(mouse_point.x - drag_mouse_offset_x_);
    pannel_y_ = static_cast<int>(mouse_point.y - drag_mouse_offset_y_);
  }
  if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    dragging_ = false;
  }
}

void Pannel::HandleUserEvent() {
  for (ToolId i = 0; i < kMaxToolCount; i++) {
    if (!tool_slots_[i]) {
      continue;
    }
    // Handle checkboxes.
    bool chbox_active_last = tool_ckbox_active_last_state_[i];
    bool chbox_active = tool_ckbox_active_state_[i];
    if (chbox_active && !chbox_active_last) {
      // Active
      DLOG_F(INFO, "Active tool {}", i);
      tool_ckbox_cb_[i](i, true);
    } else if (!chbox_active && chbox_active_last) {
      // Inactive
      DLOG_F(INFO, "Inactive tool {}", i);
      tool_ckbox_cb_[i](i, false);
    }
    tool_ckbox_active_last_state_[i] = chbox_active;
    // Handle buttons.
    if (tool_btn_update_state_[i]) {
      tool_btn_update_state_[i] = false;
      DLOG_F(INFO, "Update tool {}", i);
      tool_btn_cb_[i](i);
    }
  }
}

}  // namespace gui
}  // namespace ra2overlay
