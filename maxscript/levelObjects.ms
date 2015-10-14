fileIn "constants.ms"
fileIn "levelLogic.ms"

persistent global VINE_meleeMasterMesh = undefined
persistent global VINE_flareMasterMesh = undefined
persistent global VINE_sniperMasterMesh = undefined
persistent global VINE_TrampolineMasterMesh = undefined
persistent global VINE_CannonMasterMesh = undefined
persistent global VINE_LianaMasterMesh = undefined
persistent global VINE_CreepMasterMesh = undefined
persistent global VINE_CheckpointMasterMesh = undefined
persistent global VINE_VinedettaMasterMesh = undefined
persistent global VINE_SpawnPoint = undefined
persistent global VINE_PickupHealthMasterMesh = undefined
persistent global VINE_PickupEnergyMasterMesh = undefined
persistent global VINE_PickupInvencibleMasterMesh = undefined
persistent global VINE_PickupCoinMasterMesh = undefined
persistent global VINE_PickupCollectibleMasterMesh = undefined

persistent global VINE_objID
	
__VINE_OBJECTS_updating_1 = false

fn VINE_Object_updateCheck version = (
	if version == 1 then 
		if not __VINE_OBJECTS_updating_1 then (
			deleteLayer "MASTER_OBJECTS"
			__VINE_OBJECTS_updating_1 = true
		)
)

fn VINE_getObjectMesh maxFile obj = (
	local prevSelection = $
	clearSelection()
	local maxPath = VINE_meshesPath+"objects\\"+maxFile
	local isOK = mergemaxFile maxPath #(obj) \
		#select #noRedraw \
		#mergeDups #useMergedMtlDups
	
	if not isOk then (
		format "Failed to load %\n%" maxPath \
			"Did you forget to set the proper path in the options?\n"
		return undefined
	)
	local ret = $
	
	if prevSelection != undefined
		then select prevSelection
	return ret
)


fn addToObjectLayer obj = (
	with redraw off (
		local l = LayerManager.getLayerFromName "MASTER_OBJECTS"
		if l == undefined
			then l = LayerManager.newLayerFromName "MASTER_OBJECTS"
		l.isFrozen = false
		l.addNode obj
		l.isHidden = true
		l.isFrozen = true
		l.isFrozen = true
	)
)
	
plugin Helper VINE_Enemy
name:"Enemy" 
classID:#(TAG_level_class, TAG_level_enemy) 
category:"VINE" 
extends:dummy
version:2
invisible:true
(
	on update do VINE_Object_updateCheck version
	
	parameters persist (
		masterMesh type:#node animatable:false
	)
	
	on getDisplayMesh do if not isGone masterMesh then return masterMesh.mesh
	
	tool create ( 
		on mousePoint click do (
			viewTM = getCPTM()
			nodeTM = (transMatrix worldPoint) * (inverse viewTM)
			#stop 
		)
	)
)

plugin Helper VINE_Melee
name:"Melee Enemy" 
classID:#(TAG_level_class, TAG_level_melee) 
category:"VINE" 
extends:VINE_Enemy
version:2
(	
	on update do VINE_Object_updateCheck version
	
	fn loadMasterMesh = (
		if isGone VINE_meleeMasterMesh then (
				VINE_meleeMasterMesh = VINE_getObjectMesh "meleeMalla.max" "meleeEnemy"
				addToObjectLayer VINE_meleeMasterMesh
		)
		if delegate.masterMesh == undefined
			then delegate.masterMesh = VINE_meleeMasterMesh
	)
	
	on create do loadMasterMesh()
	
	on attachedToNode n do (
		loadMasterMesh()
		n.wirecolor = COLOR_melee
	)
)

plugin Helper VINE_Flare
name:"Flare Enemy" 
classID:#(TAG_level_class, TAG_level_flare) 
category:"VINE" 
extends:VINE_Enemy
version:2
(	
	on update do VINE_Object_updateCheck version
	
	fn loadMasterMesh = (
		if isGone VINE_flareMasterMesh then ( 
				VINE_flareMasterMesh = VINE_getObjectMesh "meleeMalla.max" "meleeEnemy"
				addToObjectLayer VINE_flareMasterMesh
		)
		if delegate.masterMesh == undefined
			then delegate.masterMesh = VINE_flareMasterMesh
	)
	
	on create do loadMasterMesh()
	
	on attachedToNode n do (
		loadMasterMesh()
		n.wirecolor = COLOR_flare
	)
)

plugin Helper VINE_Sniper
name:"Sniper Enemy" 
classID:#(TAG_level_class, TAG_level_sniper) 
category:"VINE" 
extends:VINE_Enemy
version:3
invisible:true
(
	on update do VINE_Object_updateCheck version
	
	fn loadMasterMesh = (
		if isGone VINE_sniperMasterMesh then ( 
				VINE_sniperMasterMesh = VINE_getObjectMesh "meleeMalla.max" "meleeEnemy"
				addToObjectLayer VINE_sniperMasterMesh
		)
		if delegate.masterMesh == undefined
			then delegate.masterMesh = VINE_sniperMasterMesh
	)
	
	on create do loadMasterMesh()
	
	on attachedToNode n do (
		loadMasterMesh()
		n.wirecolor = COLOR_sniper
	)
)

plugin Helper VINE_Wildcard
name:"Wildcard" 
classID:#(TAG_level_class, TAG_level_wildcard) 
category:"VINE" 
extends:dummy
version:2
invisible:false
(
	local thisNode
	local taggedNode
	
	parameters persist (
		masterMesh type:#node animatable:false
		selfId type:#integer animatable:false default:-1
		taggedID type:#integer animatable:false default:-1
	)
		
	fn getTaggedNode reload:false = (
		if reload then taggedNode = undefined
		if taggedID != -1 then (
			if taggedNode==undefined then
				for obj in $*
				where classOf obj == VINE_Piece
				do if obj.selfId == taggedID then (
					taggedNode = obj
					thisNode.parent = obj
					break
				)
			if taggedNode == undefined then (
				taggedId = -1
				format "Tagged node missing!"
			)
		)
	)
	
	fn getThisNode = (
		if thisNode==undefined then
			for obj in $*
			where classOf obj == VINE_Wildcard
			do if obj.selfId == selfId then (
				thisNode = obj
				break
			)
	)
	
	rollout wildcardRO "Wildcard" width:160 height:128
	(
		edittext tag1TXT "Tag 1:" pos:[13,7] width:136 height:17
		edittext tag2TXT "Tag 2:" pos:[13,29] width:136 height:17
		edittext tag3TXT "Tag 3:" pos:[13,51] width:136 height:17
		button tagParent "Tag parent" pos:[8,72] width:144 height:28
		label taggedLBL "No piece tagged" pos:[8,104] width:140 height:16
		
		fn reloadTag = (
			if taggedNode != undefined do (
				taggedLBL.text = "Tagged " + (taggedNode.name)
			)
		)
		
		fn findPiece a = (
			if classof a == VINE_Piece then return a
			else (
				for b in a.children do (
					local c = findPiece b
					if c != undefined then return c
				)
				return undefined
			)
		)
		
		on wildcardRO open do reloadTag()
		
		on tagParent pressed do (
			if thisNode == undefined then getThisNode()
			if thisNode.parent == undefined then (
				taggedID = -1
				taggedLBL.text = "No parent piece to tag!"
			) else (
				local piece = findPiece thisNode.parent
				if piece == undefined then (
					taggedLBL.text = "No piece found in parent!"
				) else (
					taggedID = piece.selfId
					getTaggedNode reload:true ()
					reloadTag()
				)
			)
		)
	)

	parameters objectParams rollout:wildcardRO (
		tag1 type:#string animatable:false ui:tag1TXT default:""
		tag2 type:#string animatable:false ui:tag2TXT default:""
		tag3 type:#string animatable:false ui:tag3TXT default:""
	)
	
	on load do (
		getThisNode()
		getTaggedNode()
	)
	
	on attachedToNode n do (
		n.wirecolor = yellow
		thisNode = n
	)
	
	on getDisplayMesh do (
		if isGone VINE_VinedettaMasterMesh then ( 
				VINE_VinedettaMasterMesh = VINE_getObjectMesh "vinedetta.max" "vinedetta"
				addToObjectLayer VINE_VinedettaMasterMesh
		)
		return VINE_VinedettaMasterMesh.mesh
	)
		
	tool create ( 
		on mousePoint click do (
			viewTM = getCPTM()
			nodeTM = (transMatrix worldPoint) * (inverse viewTM)
			#stop 
		)
	)
)

plugin Helper VINE_Object
name:"Game object" 
classID:#(TAG_level_class, TAG_level_object) 
category:"VINE" 
extends:dummy
version:2
invisible:true
(
	on update do VINE_Object_updateCheck version
	
	local thisNode
	
	parameters persist (
		masterMesh type:#node animatable:false
		selfId type:#integer animatable:false default:-1
	)
	
	fn setSelfId = (
		if selfID == -1 then (
			if VINE_objID == undefined then VINE_objID = 0
			VINE_objID += 1
			selfID = VINE_objID
		)
	)
	
	parameters objectParams rollout:objectParamsRO (
		transformed type:#boolean animatable:false ui:transformedCHK default:false
		
		on transformed set b do 
			if b
				then (if thisNode != undefined then thisNode.wirecolor = COLOR_gameObject_transformed)
				else (if thisNode != undefined then thisNode.wirecolor = COLOR_gameObject)
	)
	
	rollout objectParamsRO "Game object" (
		checkbox transformedCHK "Already transformed"
	)
	
	on getDisplayMesh do (
		if not isGone masterMesh then return masterMesh.mesh
		)
		
	tool create ( 
		on mousePoint click do (
			viewTM = getCPTM()
			nodeTM = (transMatrix worldPoint) * (inverse viewTM)
			#stop 
		)
	)
)

global VINE_pickupTypes = #("Health", "Energy", "Invencible","Score","Collectible")
plugin Helper VINE_Pickup
name:"Pickup" 
classID:#(TAG_level_class, TAG_level_pickup) 
category:"VINE" 
extends:VINE_Object
version:2
(	
	on update do VINE_Object_updateCheck version
		
	on load do
		if delegate.thisNode==undefined then
			for obj in $*
				where
					classOf obj == VINE_Pickup
				do if obj.delegate.selfID == delegate.selfID then (
					delegate.thisNode = obj
					break
				)
	
	parameters pickupParams rollout:pickupParams_RO (
		type type:#integer animatable:false ui:ddl1 default:1
		strength type:#integer animatable:false ui:spn1
	)
	
	function getType = return VINE_pickupTypes[type]
	
	rollout pickupParams_RO "Pickup"
	(
		dropDownList ddl1 "Type" items:VINE_pickupTypes selection:1
		spinner spn1 "Stength" range:[0,100,10] default:10
		
		on ddl1 selected sel do (
			case sel of (
				1: (--health
					spn1.value = 10
					strength   = 10
				)
				2: (--energy
					spn1.value = 10
					strength   = 10
				)
				3: (--Invencible
					spn1.value = 10
					strength   = 10
				)
				4: (--Score
					spn1.value = 1
					strength   = 1
				)
				5: (--Collectible
					spn1.value = 0
					strength   = 0
				)
			)
		)
	)
	
	fn loadMasterMesh = (
		if isGone VINE_PickupHealthMasterMesh then ( 
			format "HELLO"
				VINE_PickupHealthMasterMesh = VINE_getObjectMesh "pickup.max" "health"
				addToObjectLayer VINE_PickupHealthMasterMesh
		)
		if isGone VINE_PickupEnergyMasterMesh then ( 
				VINE_PickupEnergyMasterMesh= VINE_getObjectMesh "pickup.max" "energy"
				addToObjectLayer VINE_PickupEnergyMasterMesh
		)
		if isGone VINE_PickupInvencibleMasterMesh then ( 
				VINE_PickupInvencibleMasterMesh = VINE_getObjectMesh "pickup.max" "invencible"
				addToObjectLayer VINE_PickupInvencibleMasterMesh
		)
		if isGone VINE_PickupCoinMasterMesh then ( 
				VINE_PickupCoinMasterMesh = VINE_getObjectMesh "pickup.max" "coin"
				addToObjectLayer VINE_PickupCoinMasterMesh
		)
		if isGone VINE_PickupCollectibleMasterMesh then ( 
				VINE_PickupCollectibleMasterMesh = VINE_getObjectMesh "pickup.max" "collectible"
				addToObjectLayer VINE_PickupCollectibleMasterMesh
		)
		if delegate.masterMesh == undefined
			then delegate.masterMesh = VINE_PickupHealthMasterMesh
	)
	
	on getDisplayMesh do (
		loadMasterMesh()
		case type of (
			1: return VINE_PickupHealthMasterMesh.mesh
			2: return VINE_PickupEnergyMasterMesh.mesh
			3: return VINE_PickupInvencibleMasterMesh.mesh
			4: return VINE_PickupCoinMasterMesh.mesh
			5: return VINE_PickupCollectibleMasterMesh.mesh
			default: return delegate.masterMesh.mesh
		)
	)
	
	on create do loadMasterMesh()
	
	on attachedToNode n do (
		loadMasterMesh()
		n.wirecolor = COLOR_gameObject
		delegate.thisNode = n
		delegate.setSelfId()
	)
)

plugin Helper VINE_Trampoline
name:"Trampoline" 
classID:#(TAG_level_class, TAG_level_trampoline) 
category:"VINE" 
extends:VINE_Object
version:2
(	
	on update do VINE_Object_updateCheck version
		
	on load do
		if delegate.thisNode==undefined then
			for obj in $*
				where
					classOf obj == VINE_Trampoline
				do if obj.delegate.selfID == delegate.selfID then (
					delegate.thisNode = obj
					break
				)
	
	fn loadMasterMesh = (
		if isGone VINE_TrampolineMasterMesh then ( 
			VINE_TrampolineMasterMesh = VINE_getObjectMesh "trampoline.max" "trampoline"
			addToObjectLayer VINE_TrampolineMasterMesh
		)
		if delegate.masterMesh == undefined
			then delegate.masterMesh = VINE_TrampolineMasterMesh
	)
	
	on create do loadMasterMesh()
	
	on attachedToNode n do (
		loadMasterMesh()
		n.wirecolor = COLOR_gameObject
		delegate.thisNode = n
		delegate.setSelfId()
	)
)

plugin Helper VINE_Cannon
name:"Cannon" 
classID:#(TAG_level_class, TAG_level_cannon) 
category:"VINE" 
extends:VINE_Object
version:3
(	
	on update do VINE_Object_updateCheck version

	on load do
		if delegate.thisNode==undefined then
			for obj in $*
				where
					classOf obj == VINE_Cannon
				do if obj.delegate.selfID == delegate.selfID then (
					delegate.thisNode = obj
					break
				)
				
	parameters pBlock rollout:params (
		fovH type:#float animatable:false ui:fovHSPN default:60
		fovV type:#float animatable:false ui:fovVSPN default:60
		impulse type:#float animatable:false ui:impulseSPN default:17
	)
	
	rollout params "Enemy" (
		spinner fovHSPN "FovV:" range:[0,89, 60] type:#float
		spinner fovVSPN "FovH:" range:[0,89, 60] type:#float
		spinner impulseSPN "Impulse:" range:[0,50, 10] type:#float
	)
	
	fn loadMasterMesh = (
		if isGone VINE_CannonMasterMesh then ( 
			VINE_CannonMasterMesh = VINE_getObjectMesh "cannon.max" "cannon"
			addToObjectLayer VINE_CannonMasterMesh
		)
		if delegate.masterMesh == undefined
			then delegate.masterMesh = VINE_CannonMasterMesh
	)
	
	on create do loadMasterMesh()
	
	on attachedToNode n do (
		loadMasterMesh()
		n.wirecolor = COLOR_gameObject
		delegate.thisNode = n
		delegate.setSelfId()
	)
)

global LIANA_LENGTH = 1.25
plugin Helper VINE_Liana
name:"Liana" 
classID:#(TAG_level_class, TAG_level_liana) 
category:"VINE" 
extends:VINE_Object
version:3
(	
	on update do VINE_Object_updateCheck version
	
	on load do
		if delegate.thisNode==undefined then
			for obj in $*
				where
					classOf obj == VINE_Liana
				do if obj.delegate.selfID == delegate.selfID then (
					delegate.thisNode = obj
					break
				)	
	
	parameters pBlock rollout:params (
		links type:#integer animatable:false ui:linksSPN default:20
		limitX type:#float animatable:false ui:limitXSPN default:60
		limitZ type:#float animatable:false ui:limitZSPN default:60
	)
	
	rollout params "Liana" width:163 height:86
	(
		spinner limitXSPN "Angle X:" range:[5,90,45] scale:5 type:#float
		spinner limitZSPN "Angle Z:" range:[5,90,45] scale:5 type:#float
		spinner linksSPN "Links:" range:[2,40,10] scale:1 type:#integer
		label lengthLabel "---"
		
		function updateLabel = (
			lengthLabel.text = "Length: "+(((linksSPN.value as float)*LIANA_LENGTH) as string)+"m"
		)
		
		on params open do
			if links != undefined
			then lengthLabel.text = updateLabel()
		on linksSPN changed n do updateLabel()
	)
	
	
	fn loadMasterMesh = (
		if isGone VINE_LianaMasterMesh then ( 
				VINE_LianaMasterMesh = VINE_getObjectMesh "liana.max" "liana"
				addToObjectLayer VINE_LianaMasterMesh
		)
		if delegate.masterMesh == undefined
			then delegate.masterMesh = VINE_LianaMasterMesh
	)
	
	on create do loadMasterMesh()
	
	on attachedToNode n do (
		loadMasterMesh()
		n.wirecolor = COLOR_gameObject
		delegate.thisNode = n
		delegate.setSelfId()
	)
)

plugin Helper VINE_Creep
name:"Creep" 
classID:#(TAG_level_class, TAG_level_creep) 
category:"VINE" 
extends:VINE_Object
version:2
(	
	on update do VINE_Object_updateCheck version

	on load do
		if delegate.thisNode==undefined then
			for obj in $*
				where
					classOf obj == VINE_Creep
				do if obj.delegate.selfID == delegate.selfID then (
					delegate.thisNode = obj
					break
				)	
				
	rollout params "Creep"
	(
		edittext parentTXT "Plane:" enabled:false
		button refreshBTN "Refresh"
		
		fn showParent = (
			if not isGone delegate.thisNode then (
			if not isGone delegate.thisNode.parent then (
				local parent = delegate.thisNode.parent
				if classof parent != Plane
					then parentTXT.text = "!~"+((classof parent) as string)+" != Plane!"
					else parentTXT.text = "$"+parent.name
			)) else parentTXT.text = "!~ No Plane parent"
		)
		
		on params open do showParent()
		on refreshBTN pressed do showParent()
	)
	
	fn loadMasterMesh = (
		if isGone VINE_CreepMasterMesh then ( 
				VINE_CreepMasterMesh = VINE_getObjectMesh "marker.max" "marker"
				addToObjectLayer VINE_CreepMasterMesh
		)
		if delegate.masterMesh == undefined
			then delegate.masterMesh = VINE_CreepMasterMesh
	)
	
	on attachedToNode n do (
		loadMasterMesh()
		delegate.thisNode = n
		n.wirecolor = COLOR_gameObject
		delegate.setSelfId()
	)
)

plugin Helper VINE_Checkpoint
name:"Checkpoint" 
classID:#(TAG_level_class, TAG_level_checkpoint) 
category:"VINE" 
extends:VINE_Object
version:2
(
	on update do VINE_Object_updateCheck version

	on load do
		if delegate.thisNode==undefined then
			for obj in $*
				where
					classOf obj == VINE_Checkpoint
				do if obj.delegate.selfID == delegate.selfID then (
					delegate.thisNode = obj
					break
				)	
				
	fn findOtherCheckpoint = (
		local others =
			for a in $*
				where (classof a == VINE_Checkpoint) \
				and (a.delegate.thisNode != delegate.thisNode)
				collect a
		if others.count == 0
			then return undefined
			else return others[1]
	)
	
	parameters checkpointParams rollout:checkpointParams_RO (
		isSpawn type:#boolean animatable:false ui:spawnCHECK
		order type:#integer animatable:false ui:orderSPN
		
		on isSpawn set b do (
			if b then (				
				local prev = VINE_SpawnPoint
				VINE_SpawnPoint = this
				if prev != undefined and prev.delegate.thisNode != this.delegate.thisNode
					then prev.isSpawn = false
			) else (				
				if VINE_SpawnPoint != undefined then (
					if VINE_SpawnPoint.delegate.thisNode == this.delegate.thisNode then (
						local other = findOtherCheckpoint()
						if other != undefined
							then other.isSpawn = true
						VINE_SpawnPoint = other
					)
				)
			)
		)
	)
	
	fn createTrigger = (
		if not isGone delegate.thisNode then (
			if not isGone delegate.thisNode.parent \
			and classof delegate.thisNode.parent == VINE_CheckPointTrigger
			then (delete delegate.thisNode.parent; delegate.thisNode.parent = undefined)
			
			local trigger = VINE_CheckPointTrigger()
			trigger.position = delegate.thisNode.position
			trigger.width = 2
			trigger.height = 3
			trigger.length = 2
			delegate.thisNode.parent = trigger
			select trigger
			)
		)
	
	rollout checkpointParams_RO "CheckPoint"	 (
		checkbox spawnCHECK "Level spawn point"
		spinner orderSPN "Order number:" type:#integer range:[0,100,10] default:0
	)
		
	rollout checkpointTrigger_RO "Trigger" width:160 height:64
	(
		edittext parentTXT "" pos:[8,7] width:141 height:17 enabled:false
		button refreshBTN "Refresh" pos:[104,32] width:48 height:21
		button selectBTN "Select" pos:[56,32] width:40 height:21
		button createTriggerBTN "Create" pos:[8,32] width:40 height:21
		
		fn showParent = (
			if not isGone delegate.thisNode then (
			if not isGone delegate.thisNode.parent then (
				local parent = delegate.thisNode.parent
				if classof parent != VINE_CheckPointTrigger
					then parentTXT.text = "!~"+((classof parent) as string)+" != VINE_CheckPointTrigger!"
					else parentTXT.text = "$"+parent.name
			)) else parentTXT.text = "!~ No CheckPointTrigger parent"
		)
		
		on checkpointTrigger_RO open do
			showParent()
		on refreshBTN pressed do
			showParent()
		on createTriggerBTN pressed do
		(
					createTrigger()
					showParent()
				)
		on selectBTN pressed do
			showParent()
		on selectBTN pressed do
			if not isGone delegate.thisNode then (
			if not isGone delegate.thisNode.parent then (
				select delegate.thisNode.parent
			))
	)
	
	fn loadMasterMesh = (
		if isGone VINE_CheckpointMasterMesh then ( 
				VINE_CheckpointMasterMesh = VINE_getObjectMesh "arbol.max" "arbol"
				addToObjectLayer VINE_CheckpointMasterMesh
		)
		if isGone VINE_VinedettaMasterMesh then ( 
				VINE_VinedettaMasterMesh = VINE_getObjectMesh "vinedetta.max" "vinedetta"
				addToObjectLayer VINE_VinedettaMasterMesh
		)
		if delegate.masterMesh == undefined
			then delegate.masterMesh = VINE_CheckpointMasterMesh
	)
	
	
	on getDisplayMesh do (
		loadMasterMesh()
		if isSpawn then (
			if VINE_VinedettaMasterMesh != undefined
				then return VINE_VinedettaMasterMesh.mesh
			else if delegate.masterMesh != undefined
				then return delegate.masterMesh.mesh
		) else if delegate.masterMesh != undefined
			then return delegate.masterMesh.mesh
	)
		
	on create do loadMasterMesh()
	
	on attachedToNode n do (
		loadMasterMesh()
		n.wirecolor = COLOR_gameObject
		delegate.thisNode = n
		if isGone VINE_SpawnPoint or isGone Vine_SpawnPoint.delegate.thisNode
			then isSpawn = true
		delegate.setSelfId()
	)
	
	on detachedFromNode n do (
		if VINE_SpawnPoint != undefined and VINE_SpawnPoint.delegate ==  this.delegate then (
			VINE_SpawnPoint = findOtherCheckpoint()
			if VINE_SpawnPoint != undefined then (
				VINE_SpawnPoint.isSpawn = true
			)
		)
		delegate.thisNode = undefined
	)
)

