void ScriptInvoker_LockingDataMethod(BGONE_LockingData_BASE lockingData);
typedef func ScriptInvoker_LockingDataMethod;
typedef ScriptInvokerBase<ScriptInvoker_LockingDataMethod> ScriptInvoker_LockingDataEntity;

class BGONE_LockingData_BASE
{
	vector lockingPos;
	float lockingProgress;
}

[BaseContainerProps()]
class BGONE_LockType_Base : ScriptAndConfig
{
	protected bool m_bIsLocking = false;
	protected IEntity m_eLauncher;
	
	// Events for notifying the launcher about our lock state, returning the entity we are aquiring / locked
	protected ref ScriptInvoker_LockingDataEntity m_OnLockStartAquire;
	protected ref ScriptInvoker_LockingDataEntity m_OnLockAquired;
	protected ref ScriptInvokerVoid m_OnLockLost;
	
	// Need to work out better way to do inheritance / casting so the base methods make sense.
	protected ref BGONE_TargetData m_cTargetData;
	protected ref BGONE_LockingData_BASE m_eLockingData;
	
	protected float m_fLockDuration;
	
	
	ScriptInvoker_LockingDataEntity GetOnLockStartAquire()
	{
		if(!m_OnLockStartAquire)
			m_OnLockStartAquire = new ScriptInvoker_LockingDataEntity();
		
		return m_OnLockStartAquire;
	}
	
	ScriptInvoker_LockingDataEntity GetOnLockAquired()
	{
		if(!m_OnLockAquired)
			m_OnLockAquired = new ScriptInvoker_LockingDataEntity();
		
		return m_OnLockAquired;
	}
	
	ScriptInvokerVoid GetOnLockLost()
	{
		if(!m_OnLockLost)
			m_OnLockLost = new ScriptInvokerVoid();
		
		return m_OnLockLost;
	}
	
	void InitLockType(IEntity launcher)
	{
		m_eLauncher = launcher;
	}
	
	void StartLock()
	{
		m_eLockingData = new BGONE_LockingData_BASE();
		m_eLockingData.lockingProgress = 0;
		m_bIsLocking = true;
		m_fLockDuration = 0;
		return;
	}
	BGONE_LockingData_BASE UpdateLock(float timeSlice)
	{
		if(!m_bIsLocking)
			return null;
		
		if(timeSlice == 0)
			return null;
		
		m_fLockDuration += timeSlice;
		
		return m_eLockingData;
	}
	void StopLock()
	{
		m_bIsLocking = false;
		return;
	}
	
	BGONE_TargetData GetCurrentTargetData()
	{
		return m_cTargetData;
	}
	
	void PlayLockOnAuido(float lockingProgress)
	{
		return;
	}
	void TerminateLockOnAudio()
	{
		return;
	}
	
	// Returns the direction where we are looking to aquire a lock.
	protected array<vector> GetAimDirAndPosOfLauncher(IEntity launcher)
	{
		SCR_2DPIPSightsComponent pipSight = ArmaReforgerScripted.GetCurrentPIPSights();
		Turret turret = Turret.Cast(m_eLauncher.GetParent());
		
		vector mat[4];
		if(pipSight)
		{ 
			SCR_PIPCamera pipCam = pipSight.GetPIPCamera();
			pipCam.GetWorldCameraTransform(mat);
		}
		else if(turret)
		{
			TurretControllerComponent controller = TurretControllerComponent.Cast(turret.FindComponent(TurretControllerComponent));
			BaseCompartmentSlot slot = controller.GetCompartmentSlot();
			SCR_ChimeraCharacter shooter  = SCR_ChimeraCharacter.Cast(slot.GetOccupant());
			if(!shooter)
				return {Vector(0,0,0), Vector(0,0,0)};
			
			shooter.GetWorld().GetCurrentCamera(mat);
		}
		else
		{
			SCR_ChimeraCharacter shooter = SCR_ChimeraCharacter.Cast(launcher.GetRootParent());
			if(!shooter)
				return {Vector(0,0,0), Vector(0,0,0)};
			
			shooter.GetWorld().GetCurrentCamera(mat);
		}
		
		vector aimDir = mat[2];
		vector aimPos = mat[3];
		ref array<vector> dirAndPos = {aimDir, aimPos};
		return dirAndPos;
	}
	
	protected void LockStartAquire(BGONE_LockingData_BASE lockingData) 
	{
		if(m_OnLockStartAquire)
			m_OnLockStartAquire.Invoke(lockingData);
	}
	
	protected void LockAquired(BGONE_LockingData_BASE lockingData) 
	{
		if(m_OnLockAquired)
			m_OnLockAquired.Invoke(lockingData);
	}
	
	protected void LockLost() 
	{
		if(m_OnLockLost)
			m_OnLockLost.Invoke();
	}
	
};
