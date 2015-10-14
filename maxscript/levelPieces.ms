fileIn "constants.ms"
fileIn "tools.ms"
fileIn "levelVars.ms"

persistent global VINE_pieceDictionary = Dictionary()

__VINE_PIECE_updating_1 = false

plugin Helper VINE_Piece
name:"Piece" 
classID:#(TAG_level_class, TAG_level_piece)
category:"VINE"
invisible:true
version:2
(
	parameters persist (
		pieceKit type:#string animatable:false default:"###K"
		pieceName type:#string animatable:false default:"###N"
		pieceFileName type:#string animatable:false default:"###F"
		collisionMesh type:#node animatable:false
		thisObject type:#node animatable:false
		selfID type:#integer animatable:false default:-1
		loaded type:#boolean animatable:false
		grp type:#node animatable:false
	)
	local	self
	local reload = false

	fn bakeClassName = (
		return pieceKit+"\\"+pieceFileName
	)
	
	fn getNewId = (
		local maxId = -1
		for piece in $* where classof piece == VINE_Piece do
			if piece.selfId > maxId then maxId = piece.selfId
		selfId = maxId+1
	)
	
	-- Loads a piece and adds it to VINE_pieceDictionary. returns it as #(piece, collision) 
	fn loadMasterMesh = (
		local classname = bakeClassName()
		
		--Delete the previous instance
		if VINE_pieceDictionary.hasItem className then (
			local prev = VINE_pieceDictionary.eraseItem className
			if prev != undefined then
				for o in prev do
					if not isGone o then delete o
		)
		
		--Load the new
		clearSelection()
		local isOK = mergemaxFile \
			(VINE_meshesPath+"kits\\"+className+".max") \
			#("PIECE", "COLLISION") \
			#select #noRedraw \
			#mergeDups #useMergedMtlDups  
		
		if not isOk then (
			format "Failed to load %kits/%.max\n%" VINE_meshesPath className \
				"Did you forget to set the proper path in the options?"
			return undefined
		)
		
		--New Objects are selected. Tell them apart
		local piece
		local col
		
		for obj in $ do (
			case obj.name of (
				"PIECE": piece = obj
				"COLLISION": col = obj
			)
		)
		clearSelection()
		
		local srcName = "SRC:" + className
		if piece == undefined then (
			messageBox  "ERROR: no object PIECE found." title:("Loading piece "+classname)
			return undefined
		) else (
			if isGroupHead piece then (
				messageBox "ERROR: group pieces no longer supported." title:("Loading piece "+classname)
				return undefined
			) else (
				piece.name = srcName
			)
		)
		if col == undefined then (
			format "WARNING: no object COLLISION found.\n"
		) else (
			col.parent = piece
			col.name = srcName + ".col"
			col.material = VINE_collisionMaterial
		)
		
		local ret = #(piece, col)
		
		VINE_pieceDictionary.addItem className ret
		
		format "Loaded piece: %\n" className
		
		local l = LayerManager.getLayerFromName "MASTER_PIECES"
		if l == undefined
			then l = LayerManager.newLayerFromName "MASTER_PIECES"
		l.isFrozen = false
		l.addNode piece
		if col != undefined then l.addNode col
		l.isHidden = true
		l.isFrozen = true
		
		return ret
	)
	
	fn loadPieceMesh reloadMaster:false = (
		if not (pieceKit == "###K" or pieceName == "###N" or pieceFileName == "###F")
		then (
			with redraw off  (
				with undo off (
					local className = bakeClassName()
					local piece = VINE_pieceDictionary.getItem className
					if reloadMaster \
						or piece == undefined \
						or isGone piece[1] \
						or (piece[2] != undefined and isDeleted piece[2]) \
					then (
						format "Loading master mesh...\n"
						piece = loadMasterMesh()
					)
					if piece == undefined then (
						format "Failed to load % (piece==undefined)\n" piece
						return undefined
					)
					
					if thisObject != undefined then (
						setGroupMember thisObject false
						delete thisObject
					)
					setGroupMember self false
					thisObject = copy piece[1]
					thisObject.name = className+".mesh"
					
					if collisionMesh != undefined then (
						setGroupMember collisionMesh false
						delete collisionMesh
					)
					
					if not isGone grp then (
						ungroup grp
					)
					
					self.name = className+".piece"
					
					thisObject.transform *= self.transform
					thisObject.wirecolor = color 255 255 255
					if not isGone piece[2] then (
						collisionMesh = copy piece[2]
						collisionMesh.name = "COLLISION"
						collisionMesh.transform *= self.transform
						collisionMesh.parent = thisObject
						collisionMesh.material = VINE_collisionMaterial
						collisionMesh.wirecolor = VINE_collisionMaterial.selfIllumColor
						collisionMesh.isHidden = VINE_collisionToggle
					)
					self.parent = thisObject
					if not isGone collisionMesh
						then grp = group #(thisObject, self, collisionMesh)
						else grp = group #(thisObject, self)
					grp.name = className+"-"+(selfId as string)
					loaded = true
					if isTransformablePiece self
						then thisObject.wirecolor = color 0 255 0
					if isSpecialPiece self
						then thisObject.wirecolor = color 196 0 127
					return grp
				)
			)
		) else return undefined
	)
		
	fn loadIfNot = (
		if not loaded
			then return loadPieceMesh()
	)
	
	on load do (
		for obj in $*
			where classof obj == VINE_Piece
			do 
				if obj.selfID == selfID 	-- Are you me?
				then self = obj			-- Oh, then I am you!
	)
	
	on postLoad do (
		loadPieceMesh ()
	)
	
	on attachedToNode n do (
		self = n
		if selfID == -1 then getNewId()
		local g = loadIfNot()
		
		self.name = bakeClassName()+".piece"
		if g != undefined then select g
	)
	
	on clone original do (
		local className = bakeClassName()
		getNewId()
		grp.name = className+"-"+(selfId as string)
		thisObject.name = className+".mesh"
		if not isGone collisionMesh then
			collisionMesh.name = "COLLISION"
	)
	
	fn changeTo nPieceKit nPieceName nPieceFileName
		sel:true reloadMaster:false = (
		local prevTransform
		if isGone thisObject
			then prevTransform = self.transform
			else prevTransform = thisObject.transform
		pieceFileName = nPieceFileName
		pieceName = nPieceName
		pieceKit = nPieceKit
		local g = loadPieceMesh reloadMaster:reloadMaster ()
		if sel then select g
		return g
	)
	
	
	on update do (
		if version == 1 then (
			--xrefs
			if not __VINE_PIECE_updating_1 then (
				VINE_pieceDictionary = Dictionary()
				deleteLayer "MASTER_PIECES"
				__VINE_PIECE_updating_1 = true
			)
			reload = true
		)
	)
)

fn VINE_LevelGarbageCollection = (
	local d = #()
	d = for obj in $*
		where classof obj == dummy \
		and isGroupHead obj \
		and obj.children.count == 0 collect obj
	local n = d.count
	if n > 0 then (
		for a in d do delete a
		format "[VINE_LevelGarbageCollection] deleted % childless groups\n" n
	)
	
	d = #()
	for obj in $* where classof obj == VINE_Piece do (
		if (obj.parent == undefined or isDeleted obj.parent) then (
			append d obj
		)
	)
	n = d.count
	if n > 0 then (
		for a in d do delete a
		format "[VINE_LevelGarbageCollection] deleted % orphan piece objects.\n" n
	)
)

-- External constructor
fn newVINE_Piece
	pieceKit pieceName pieceFileName
	sel:true
	pos:[0,0,0]
	= (
	with redraw off  (
		local piece = VINE_Piece()
		piece.self = piece
		piece.changeTo pieceKit pieceName pieceFileName sel:sel
		return piece
	)
)

fn LOAD_MACRO_VINE_ToggleCollision = (
	macroScript MACRO_VINE_ToggleCollision
		category:"VINEDETTA"
		toolTip: "Toggle collision display"
		icon:#("collision", 1)
	(
		VINE_collisionToggle = not VINE_collisionToggle
		for obj in $* where obj.name == "COLLISION" do (
			obj.isHidden = VINE_collisionToggle
		)
	)
	OK
)
LOAD_MACRO_VINE_ToggleCollision()


