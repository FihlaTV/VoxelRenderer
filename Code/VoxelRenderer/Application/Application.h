#pragma once

#include "swGUI/Core/System/GUISystem.h"

#include "Config.h"

#include "VoxelRenderer/Raycaster/IRaycaster.h"
#include "VoxelRenderer/Actors/CameraActor.h"

#include "VoxelRenderer/Application/TimeManager.h"



namespace vr
{



/**@brief Application template class.

User should implement virtual functions to use GUI.*/
class Application : public sw::gui::GUISystem
{
private:

	ConfigUPtr			m_config;
	
	IRaycasterUPtr		m_raycaster;
	CameraActor*		m_camera;
	OctreePtr			m_octree;

	ResourcePtr< RenderTargetObject >		m_svoRT;
	ResourcePtr< RenderTargetObject >		m_mainRT;

	TimeManager			m_timeManager;

protected:
public:
	explicit	Application		( int argc, char** argv, sw::gui::INativeGUI* gui );
				~Application	() = default;

protected:
	virtual	void	Initialize		() override;
	virtual void	OnInitialized	() override;
	virtual void	OnClosing		() override;
	virtual void	OnIdle			() override;


private:
	void			Update		();
	void			Render		();


private:
	///@name Initialization functions
	///@{
	void			InitCamera		();
	void			InitRaycaster	();
	void			InitOctree		();
	void			InitResources	();
	///@}

	void			Blit			( RenderTargetObject* svoRenderTarget, RenderTargetObject* mainRT );
};


}	// vr
