
[BaseContainerProps()]
class BGONE_LockType_SACLOS : BGONE_LockType_Base
{
	protected ref BGONE_TargetData m_cTargetDataSACLOS;
	
	override BGONE_TargetData UpdateLock(float timeSlice)
	{
		return null;
	}
	
	override BGONE_TargetData GetCurrentTargetData() 
	{
		m_cTargetDataSACLOS = new BGONE_TargetData();
			
		
		// Check if launcher is in a turret.
		Turret turret = Turret.Cast(m_eLauncher.GetParent());
		if(turret)
		{
			TurretControllerComponent controller = TurretControllerComponent.Cast(turret.FindComponent(TurretControllerComponent));
			BaseCompartmentSlot slot = controller.GetCompartmentSlot();
			m_cTargetDataSACLOS.shooterRplId = Replication.FindId(slot.GetOccupant());
		}
		else
		{
			m_cTargetDataSACLOS.shooterRplId = Replication.FindId(m_eLauncher.GetRootParent());
		}
		
		return m_cTargetDataSACLOS;
	}
	
	override void StartLock()
	{
		return;
	}
	
	override void StopLock()
	{
		return;
	}
	
};
