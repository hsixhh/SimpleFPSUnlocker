#pragma once
#include <imgui.h>




namespace ImGui {

    // Based on the code by krys-spectralpixel (https://github.com/krys-spectralpixel), posted here: https://github.com/ocornut/imgui/issues/261
    /* pOptionalHoveredIndex: a ptr to an optional int that is set to -1 if no tab label is hovered by the mouse.
     * pOptionalItemOrdering: an optional static array of unique integers from 0 to numTabs-1 that maps the tab label order. If one of the numbers is replaced by -1 the tab label is not visible (closed). It can be read/modified at runtime.
     * allowTabReorder (requires pOptionalItemOrdering): allows tab reordering through drag and drop (it modifies pOptionalItemOrdering).
     * allowTabClosingThroughMMB (requires pOptionalItemOrdering): closes the tabs when MMB is clicked on them, by setting the tab value in pOptionalItemOrdering to -1.
     * pOptionalClosedTabIndex (requires allowTabClosingThroughMMB): out variable (int pointer) that returns the index of the closed tab in last call or -1.
     * pOptionalClosedTabIndexInsideItemOrdering: same as above, but index of the pOptionalItemOrdering array.
    */
    IMGUI_API bool TabLabels(int numTabs, const char** tabLabels, int& selectedIndex, const char** tabLabelTooltips = NULL, bool wrapMode = true, int* pOptionalHoveredIndex = NULL, int* pOptionalItemOrdering = NULL, bool allowTabReorder = true, bool allowTabClosingThroughMMB = true, int* pOptionalClosedTabIndex = NULL, int* pOptionalClosedTabIndexInsideItemOrdering = NULL) {
        ImGuiStyle& style = ImGui::GetStyle();

        const ImVec2 itemSpacing = style.ItemSpacing;
        const ImVec4 color = style.Colors[ImGuiCol_Button];
        const ImVec4 colorActive = style.Colors[ImGuiCol_ButtonActive];
        const ImVec4 colorHover = style.Colors[ImGuiCol_ButtonHovered];
        const ImVec4 colorText = style.Colors[ImGuiCol_Text];
        style.ItemSpacing.x = 1;
        style.ItemSpacing.y = 1;
        const ImVec4 colorSelectedTab(color.x, color.y, color.z, color.w * 0.5f);
        const ImVec4 colorSelectedTabHovered(colorHover.x, colorHover.y, colorHover.z, colorHover.w * 0.5f);
        const ImVec4 colorSelectedTabText(colorText.x * 0.8f, colorText.y * 0.8f, colorText.z * 0.6f, colorText.w * 0.8f);
        //ImGui::ClampColor(colorSelectedTabText);

        if (numTabs > 0 && (selectedIndex < 0 || selectedIndex >= numTabs)) {
            if (!pOptionalItemOrdering)  selectedIndex = 0;
            else selectedIndex = -1;
        }
        if (pOptionalHoveredIndex) *pOptionalHoveredIndex = -1;
        if (pOptionalClosedTabIndex) *pOptionalClosedTabIndex = -1;
        if (pOptionalClosedTabIndexInsideItemOrdering) *pOptionalClosedTabIndexInsideItemOrdering = -1;

        float windowWidth = 0.f, sumX = 0.f;
        if (wrapMode) windowWidth = ImGui::GetWindowWidth() - style.WindowPadding.x - (ImGui::GetScrollMaxY() > 0 ? style.ScrollbarSize : 0.f);

        static int draggingTabIndex = -1; int draggingTabTargetIndex = -1;   // These are indices inside pOptionalItemOrdering
        static ImVec2 draggingTabSize(0, 0);
        static ImVec2 draggingTabOffset(0, 0);

        const bool isMMBreleased = ImGui::IsMouseReleased(2);
        const bool isMouseDragging = ImGui::IsMouseDragging(0, 2.f);
        int justClosedTabIndex = -1, newSelectedIndex = selectedIndex;


        bool selection_changed = false; bool noButtonDrawn = true;
        for (int j = 0, i; j < numTabs; j++)
        {
            i = pOptionalItemOrdering ? pOptionalItemOrdering[j] : j;
            if (i == -1) continue;

            if (!wrapMode) { if (!noButtonDrawn) ImGui::SameLine(); }
            else if (sumX > 0.f) {
                sumX += style.ItemSpacing.x;   // Maybe we can skip it if we use SameLine(0,0) below
                sumX += ImGui::CalcTextSize(tabLabels[i]).x + 2.f * style.FramePadding.x;
                if (sumX > windowWidth) sumX = 0.f;
                else ImGui::SameLine();
            }

            if (i == selectedIndex) {
                // Push the style
                style.Colors[ImGuiCol_Button] = colorSelectedTab;
                style.Colors[ImGuiCol_ButtonActive] = colorSelectedTab;
                style.Colors[ImGuiCol_ButtonHovered] = colorSelectedTabHovered;
                style.Colors[ImGuiCol_Text] = colorSelectedTabText;
            }
            // Draw the button
            ImGui::PushID(i);   // otherwise two tabs with the same name would clash.
            if (ImGui::Button(tabLabels[i])) { selection_changed = (selectedIndex != i); newSelectedIndex = i; }
            ImGui::PopID();
            if (i == selectedIndex) {
                // Reset the style
                style.Colors[ImGuiCol_Button] = color;
                style.Colors[ImGuiCol_ButtonActive] = colorActive;
                style.Colors[ImGuiCol_ButtonHovered] = colorHover;
                style.Colors[ImGuiCol_Text] = colorText;
            }
            noButtonDrawn = false;

            if (wrapMode) {
                if (sumX == 0.f) sumX = style.WindowPadding.x + ImGui::GetItemRectSize().x; // First element of a line
            }
            else if (isMouseDragging && allowTabReorder && pOptionalItemOrdering) {
                // We still need sumX
                if (sumX == 0.f) sumX = style.WindowPadding.x + ImGui::GetItemRectSize().x; // First element of a line
                else sumX += style.ItemSpacing.x + ImGui::GetItemRectSize().x;

            }


        }

        selectedIndex = newSelectedIndex;


        // Drop tab label
        if (draggingTabTargetIndex != -1) {
            // swap draggingTabIndex and draggingTabTargetIndex in pOptionalItemOrdering
            const int tmp = pOptionalItemOrdering[draggingTabTargetIndex];
            pOptionalItemOrdering[draggingTabTargetIndex] = pOptionalItemOrdering[draggingTabIndex];
            pOptionalItemOrdering[draggingTabIndex] = tmp;
            //fprintf(stderr,"%d %d\n",draggingTabIndex,draggingTabTargetIndex);
            draggingTabTargetIndex = draggingTabIndex = -1;
        }

        // Reset draggingTabIndex if necessary
        if (!isMouseDragging) draggingTabIndex = -1;

        // Change selected tab when user closes the selected tab
        if (selectedIndex == justClosedTabIndex && selectedIndex >= 0) {
            selectedIndex = -1;
            for (int j = 0, i; j < numTabs; j++) {
                i = pOptionalItemOrdering ? pOptionalItemOrdering[j] : j;
                if (i == -1) continue;
                selectedIndex = i;
                break;
            }
        }

        // Restore the style
        style.Colors[ImGuiCol_Button] = color;
        style.Colors[ImGuiCol_ButtonActive] = colorActive;
        style.Colors[ImGuiCol_ButtonHovered] = colorHover;
        style.Colors[ImGuiCol_Text] = colorText;
        style.ItemSpacing = itemSpacing;

        return selection_changed;
    }


} // namespace ImGui