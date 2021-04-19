#include "stdafx.h"
#include "BasicUITypes.h"

#include "BasicUIRenderer.h"
#include "BasicUI.h"

namespace XYZ {
	namespace Helper {
		static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
		{
			return (pos.x + size.x > point.x &&
				pos.x		   < point.x&&
				pos.y + size.y >  point.y &&
				pos.y < point.y);
		}
	}
	bUIElement::bUIElement(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		Coords(coords),
		Size(size),
		Color(color),
		ActiveColor(color),
		Label(label),
		Name(name),
		Parent(nullptr),
		Visible(true),
		ChildrenVisible(true),
		Type(type)
	{
	}
	glm::vec2 bUIElement::GetAbsolutePosition() const
	{
		if (Parent)
		{
			return Parent->GetAbsolutePosition() + Coords;
		}
		return Coords;
	}
	uint32_t bUIElement::depth()
	{
		if (Parent)
		{
			return Parent->depth() + 1;
		}
		return 0;
	}
	bUIButton::bUIButton(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type)
	{
	}
	void bUIButton::PushQuads(bUIRenderer& renderer)
	{
		renderer.Submit<bUIButton>(*this, bUI::GetContext().Config.GetSubTexture(bUIConfig::Button));
	}
	bool bUIButton::OnMouseMoved(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			ActiveColor = bUI::GetConfig().GetColor(bUIConfig::HighlightColor);
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Hoover);
			return true;
		}
		ActiveColor = Color;
		return false;
	}
	bool bUIButton::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), Size, mousePosition))
		{
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Press);
			return true;
		}
		return false;
	}
	bool bUIButton::OnRightMousePressed(const glm::vec2& mousePosition)
	{
		return Helper::Collide(GetAbsolutePosition(), Size, mousePosition);
	}
	bUICheckbox::bUICheckbox(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color,  const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type),
		Checked(false)
	{
	}
	bUISlider::bUISlider(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color, const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color, label, name, type),
		Value(0.0f)
	{
	}
	bUIGroup::bUIGroup(const glm::vec2& coords, const glm::vec2& size, const glm::vec4& color,  const std::string& label, const std::string& name, bUIElementType type)
		:
		bUIElement(coords, size, color,  label, name, type)
	{
	}
	void bUIGroup::PushQuads(bUIRenderer& renderer)
	{
		renderer.Submit<bUIGroup>(
			*this, 
			bUI::GetContext().Config.GetSubTexture(bUIConfig::Button), 
			bUI::GetContext().Config.GetSubTexture(bUIConfig::MinimizeButton)
		);
	}
	bool bUIGroup::OnMouseMoved(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), {Size.x, ButtonSize.y}, mousePosition))
		{
			ActiveColor = bUI::GetConfig().GetColor(bUIConfig::HighlightColor);
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Hoover);
			return true;
		}
		ActiveColor = Color;
		return false;
	}
	bool bUIGroup::OnLeftMousePressed(const glm::vec2& mousePosition)
	{
		if (Helper::Collide(GetAbsolutePosition(), ButtonSize, mousePosition))
		{
			ChildrenVisible = !ChildrenVisible;
			for (auto& callback : Callbacks)
				callback(bUICallbackType::Press);
			return true;
		}
		return false;
	}
	bool bUIGroup::OnRightMousePressed(const glm::vec2& mousePosition)
	{
		return Helper::Collide(GetAbsolutePosition(), { Size.x, ButtonSize.y } , mousePosition);
	}
}