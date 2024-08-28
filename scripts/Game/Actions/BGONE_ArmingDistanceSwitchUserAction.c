class BGONE_ArmingDistanceSwitchUserAction : SCR_InspectionUserAction
{
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;
		
		BGONE_GuidedMissileLauncherComponent missileComponent = BGONE_GuidedMissileLauncherComponent.Cast(m_WeaponComponent.FindComponent(BGONE_GuidedMissileLauncherComponent));
		if (!missileComponent)
			return false;
		
		return (missileComponent && missileComponent.GetArmingDistancesCount() > 1);
	}
	
	override bool GetActionNameScript(out string outName)
	{
		BGONE_GuidedMissileLauncherComponent missileComponent = BGONE_GuidedMissileLauncherComponent.Cast(m_WeaponComponent.FindComponent(BGONE_GuidedMissileLauncherComponent));
		
		outName = "Change Arming Distance For Seeker (" + missileComponent.GetCurrentArmingDistance().ToString() + "m)";
		return true;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_WeaponComponent)
			return;
		BGONE_GuidedMissileLauncherComponent missileComponent = BGONE_GuidedMissileLauncherComponent.Cast(m_WeaponComponent.FindComponent(BGONE_GuidedMissileLauncherComponent));
		if (!missileComponent)
			return;
		
		missileComponent.CycleArmingDistance();
	}
};