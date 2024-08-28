void ScriptInvoker_OwnerChangedMethod(RplIdentity ownerId);
typedef func ScriptInvoker_OwnerChangedMethod;
typedef ScriptInvokerBase<ScriptInvoker_OwnerChangedMethod> ScriptInvoker_OwnerChanged;

[ComponentEditorProps(category: "GameScripted/Weapons/BGONE", description: "")]
class BGONE_GuidedMissileLauncherComponentClass : ScriptGameComponentClass
{
};
class BGONE_GuidedMissileLauncherComponent : ScriptGameComponent
{
	[Attribute("", UIWidgets.Object, desc: "Script Responsible For Gathering Initial Target Data To Be Sent To The Missile", category: "BGONE")]
	protected ref BGONE_LockType_Base m_eLockTypeComponent;
	
	protected IEntity m_eOwner;
	protected SCR_ChimeraCharacter m_eCurrentPlayer;
	protected EventHandlerManagerComponent m_eventHandler;
	protected InputManager m_InputManager;
	protected ref array<ref BGONE_AttackProfile_Base> m_eSupportedAttackProfiles;
	protected int m_iCurrentAttackModeIndex = 0;
	protected ref array<int> m_aAvailableArmingDistances;
	protected int m_iCurrentArmingDistanceIndex = 0;
	protected RplComponent m_RplComponent;
	protected TurretControllerComponent m_eTurretController;
	protected bool locking;
	
	protected ref BGONE_TargetData m_eLastTargetData;
	protected BGONE_GuidedMissileComponent m_eLastMissile;
	protected BGONE_GuidedMissileComponent m_eLastMissileSaclos;
	
	protected ref array<ref Shape> m_aDbgCollisionShapes;
	
	
	
	// Methods for handling ownership change and action context (de)activation.
	protected override void EOnActivate(IEntity owner)
	{
		super.EOnActivate(owner);
		
		// Check if launcher is in a turret.
		Turret turret = Turret.Cast(owner.GetParent());
		if(turret)
		{
			m_eTurretController = TurretControllerComponent.Cast(turret.FindComponent(TurretControllerComponent));
			BaseCompartmentSlot slot = m_eTurretController.GetCompartmentSlot();
			m_eCurrentPlayer = SCR_ChimeraCharacter.Cast(slot.GetOccupant());
		}
		else
		{
			m_eCurrentPlayer = SCR_ChimeraCharacter.Cast(owner.GetRootParent());
		}
		
		if(!m_eCurrentPlayer)
			return;
		
		// Update the owner to the person holding the launcher.
		Rpc(RpcAsk_GiveOwnerShip, Replication.FindOwner(m_eCurrentPlayer.GetRplComponent().Id()));
	}
	protected override void EOnDeactivate(IEntity owner)
	{
		super.EOnDeactivate(owner);
		
		if(!m_eCurrentPlayer)
			return;
	
		// Remove the OnProjectileShot if it is not already.
		if(m_eventHandler)
		{
			RemoveListeners();
		}	
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_GiveOwnerShip(RplIdentity identity)
	{
		m_RplComponent.GiveExt(identity, false);
		RegisterListeners();
		Rpc(RpcDo_GiveOwnerShip, identity);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_GiveOwnerShip(RplIdentity identity)
	{
		RegisterListeners();
	}
	
	protected override void EOnInit(IEntity owner)
	{
		if(!m_eLockTypeComponent)
		{
			Print("BGONE - Guided missile launcher initialized without a LockType component!");
			return;
		}
		m_eOwner = owner;
		m_RplComponent = RplComponent.Cast(m_eOwner.FindComponent(RplComponent));
		m_eLockTypeComponent.InitLockType(owner);
		m_InputManager = GetGame().GetInputManager();
	}
	
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		if(!m_RplComponent.IsOwner() || !m_eCurrentPlayer)
			return;
		
		bool turretAds = false;
		bool weaponAds = false;
		
		BaseWeaponComponent weaponComp = m_eCurrentPlayer.GetWeaponManager().GetCurrentWeapon();
		if(weaponComp)
			weaponAds = weaponComp.IsSightADSActive();
		
		if(m_eTurretController)
			turretAds = m_eTurretController.IsWeaponADS();
		 	
		if(!m_eventHandler || (!weaponAds && !turretAds))
		{
			if(locking)
				m_eLockTypeComponent.StopLock();
			return;
		}
			
		m_InputManager.ActivateContext("CharacterWeaponGuidedLauncher");
		
		m_eLockTypeComponent.UpdateLock(timeSlice);
	}
	protected void SetLockingState(float value, EActionTrigger reason)
	{
		if(!m_RplComponent.IsOwner())
			return;
		
		locking = reason == EActionTrigger.DOWN;
		if(locking)
			m_eLockTypeComponent.StartLock();
		else
			m_eLockTypeComponent.StopLock();
	}
	
	protected void LockStartAquire(BGONE_LockingData_BASE lockingData)
	{
		Rpc(RpcAsk_PlayLockonAudio, lockingData.lockingProgress, false);
	}
	
	protected void LockAquired(BGONE_LockingData_BASE lockingData)
	{
		Rpc(RpcAsk_PlayLockonAudio, lockingData.lockingProgress, false);
	}
	
	protected void LockLost()
	{
		Rpc(RpcAsk_PlayLockonAudio, 0, true);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_PlayLockonAudio(float lockingProgress, bool terminate)
	{
		if(terminate)
			m_eLockTypeComponent.TerminateLockOnAudio();
		else
			m_eLockTypeComponent.PlayLockOnAuido(lockingProgress);
			
		Rpc(RpcDo_PlayLockonAudio, lockingProgress, terminate);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PlayLockonAudio(float lockingProgress, bool terminate)
	{
		if(terminate)
			m_eLockTypeComponent.TerminateLockOnAudio();
		else
			m_eLockTypeComponent.PlayLockOnAuido(lockingProgress);
	}
	
	protected void OnLaunch(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{	
		if(m_RplComponent.IsRemoteProxy())
			return;
		
		m_eLastMissile = BGONE_GuidedMissileComponent.Cast(entity.FindComponent(BGONE_GuidedMissileComponent));
		if(!m_eLastMissile)
			return;
		
		// If targetdata already set from owner, use that.
		BGONE_TargetData targetData;
		if(m_eLastTargetData)
			targetData = m_eLastTargetData;
		else
		{
			targetData = m_eLockTypeComponent.GetCurrentTargetData();
			
			if(!targetData)
			return;
			
			targetData.launchPos = entity.GetOrigin();
			targetData.attackProfileIndex = m_iCurrentAttackModeIndex;
			targetData.armingDistancesIndex = m_iCurrentArmingDistanceIndex;
			if(m_eTurretController)
			targetData.turretRplId = Replication.FindId(m_eTurretController);
		}
		
		Rpc(RpcAsk_SendTargetData, BGONE_TargetData.Cast(targetData.Clone()));
	}
	
	void SaclosFix()
	{
		Rpc(RpcDo_SaclosFix);
	}
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Owner)]
	void RpcDo_SaclosFix()
	{
		vector aimDir, aimPos;
		if(m_eTurretController)
		{
			aimDir = m_eTurretController.GetCurrentSights().GetSightsDirectionUntransformed();
			aimPos = m_eTurretController.GetCurrentSights().GetSightsRearPosition();
		}
		else
		{
			aimDir = m_eCurrentPlayer.GetHeadAimingComponent().GetAimingDirectionWorld();
			aimDir[0] = -aimDir[0];
			aimPos = m_eCurrentPlayer.EyePosition();
		}
		Rpc(RpcAsk_SaclosFix, aimDir, aimPos);
	}
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Server)]
	void RpcAsk_SaclosFix(vector aimDir, vector aimPos)
	{
		if(m_eLastMissileSaclos)
		{
			m_eLastMissileSaclos.UpdateTurretAim(aimDir, aimPos);
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_SendTargetData(BGONE_TargetData targetData)
	{
		if(!m_eLastMissile)
		{
			m_eLastTargetData = targetData;
			return;
		}
		
		// Save reference to fired missile for saclos fix.
		m_eLastMissileSaclos = m_eLastMissile;
		
		m_aDbgCollisionShapes = m_eLastMissile.onLaunched(targetData, this);
		
		// Consumed last missile and targetdata, reset.
		m_eLastTargetData = null;
		m_eLastMissile = null;
	}
	
	protected void RegisterListeners()
	{
		// Register the OnProjectileShot if it is not already.
		if(!m_eventHandler)
		{
			m_eventHandler = EventHandlerManagerComponent.Cast(m_eCurrentPlayer.FindComponent(EventHandlerManagerComponent));
			if(!m_eventHandler)
			{
				Print("BGONE GuidedMissileLauncher - EventHandler Missing On Equipped Player", LogLevel.ERROR);
				return;
			}
		}
		m_eLockTypeComponent.GetOnLockStartAquire().Insert(LockStartAquire);
		m_eLockTypeComponent.GetOnLockAquired().Insert(LockAquired);
		m_eLockTypeComponent.GetOnLockLost().Insert(LockLost);
		
		m_eventHandler.RegisterScriptHandler("OnProjectileShot", this, OnLaunch);

		m_InputManager.AddActionListener("BGONELock", EActionTrigger.DOWN, SetLockingState);
		m_InputManager.AddActionListener("BGONELock", EActionTrigger.UP, SetLockingState);
	}
	
	protected void RemoveListeners()
	{
		m_eLockTypeComponent.GetOnLockStartAquire().Remove(LockStartAquire);
		m_eLockTypeComponent.GetOnLockAquired().Remove(LockAquired);
		m_eLockTypeComponent.GetOnLockLost().Remove(LockLost);
		LockLost();
		
		m_eventHandler.RemoveScriptHandler("OnProjectileShot", this, OnLaunch);
		m_eventHandler = null;
		
		m_InputManager.RemoveActionListener("BGONELock", EActionTrigger.DOWN, SetLockingState);
		m_InputManager.RemoveActionListener("BGONELock", EActionTrigger.UP, SetLockingState);
	}
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FIXEDFRAME);
	}
	
	// Methods for registering available firemodes and arming distances.
	void SetAvailableAttackProfiles(array<ref BGONE_AttackProfile_Base> attackProfiles)
	{
		m_eSupportedAttackProfiles = attackProfiles;
	}
	
	void SetAvailableArmingDistances(array<int> armingDistances)
	{
		m_aAvailableArmingDistances = armingDistances;
	}
	// Methods for selecting firemode and armingdistance on the launcher.
	int GetArmingDistancesCount() 
	{
		if(!m_aAvailableArmingDistances)
			return 0;
		
		return m_aAvailableArmingDistances.Count();
	}
	void CycleArmingDistance()
	{
		if(m_iCurrentArmingDistanceIndex < GetArmingDistancesCount() - 1)
			m_iCurrentArmingDistanceIndex ++;
		else
			m_iCurrentArmingDistanceIndex = 0;
	}
	int GetCurrentArmingDistance()
	{
		return m_aAvailableArmingDistances[m_iCurrentArmingDistanceIndex];
	}
	int GetAttackModesCount() 
	{
		if(!m_eSupportedAttackProfiles)
			return 0;
		return m_eSupportedAttackProfiles.Count();
	}
	void CycleAttackMode()
	{
		if(m_iCurrentAttackModeIndex < GetAttackModesCount() - 1)
			m_iCurrentAttackModeIndex ++;
		else
			m_iCurrentAttackModeIndex = 0;
	}
	BGONE_AttackProfile_Base GetCurrentAttackMode()
	{
		return m_eSupportedAttackProfiles[m_iCurrentAttackModeIndex];
	}
};
