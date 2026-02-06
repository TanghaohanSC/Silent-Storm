#ifndef __GRenderExecute_H_
#define __GRenderExecute_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class CTransformStack;
namespace NGScene
{
struct SLightInfo
{
	bool bNeedSet;
	CVec3 vLightColor, vGlossColor, vShadowColor, vAmbientColor;
	CVec4 vLightPos, vRadius;

	SLightInfo(): bNeedSet(false), vLightColor(VNULL3), vGlossColor(VNULL3), vLightPos(VNULL4), vRadius(1,1,1,1) {}
};

void Execute( IRender *pRender, NGfx::CRenderContext *pRC, const CTransformStack &ts, const CRenderCmdList &cl,
	const SLightInfo &lightInfo );
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif
