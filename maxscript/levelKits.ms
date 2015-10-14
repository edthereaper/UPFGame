fileIn "levelPieces.ms"
fileIn "tools.ms"
fileIn "export_mesh.ms"

global VINE_JoinedColName = "Joined Collision"

struct pieceName (
	kit = "",
	piece = "",
	variations = 1,
	fn asString = (
		local varNum = ""
		if variations > 1 then varNum = " ("+(variations as string)+")"
		return "["+ kit + "]" + piece + varNum
	)
)

rollout ROLLOUT_VINE_kits "VINE level editor piece picker" width:368 height:616
(
	dropdownList kitSelDDL "Kit" pos:[8,8] width:144 height:40 items:#("All") toolTip:"Filter by kit."
	listbox pieceList "Pieces" pos:[8,56] width:352 height:18 toolTip:"Available pieces that fill the filter criterion."
	edittext filterTxtIn "Filter:" pos:[160,32] width:168 height:16 toolTip:"Filter by name."
	bitmap pieceIconBMP "Bitmap" pos:[8,320] width:128 height:128 toolTip:"<piecename>.bmp"
	button newBTN "New" pos:[272,320] width:40 height:24 toolTip:"Create a new object of the selected type."
	button findBTN "Find" pos:[320,320] width:40 height:24 toolTip:"Select all objects of the specified kind."
	button randBTN "Rand variation" pos:[264,408] width:88 height:24 toolTip:"Randomize selected pieces, and randomize the variation spinner."
	spinner variationSPN "Variation:" pos:[168,320] width:96 height:16 range:[0,0,0] type:#integer toolTip:"Select a different variation of the selected piece."
	button levelExpBTN "Complete" pos:[16,472] width:64 height:24 enabled:true toolTip:"Export the whole level, including collision meshes."
	dropdownList objectSelDDL "Select objects" pos:[8,544] width:352 height:40 items:#("Nothing", "Pieces", "Melees", "Flares", "Snipers", "Trampolines", "Cannons", "Lianas", "Creeps", "CheckPoints") toolTip:"Select all objects in the scene that are of a certain kind"
	
	local foundPieces = Dictionary()
	
	fn getVariationCandidatesOf listElement = (
		local kits_path = VINE_meshesPath+"kits\\"
		local piece = foundPieces.getItem listElement
		local base = kits_path + piece.kit + "\\"+piece.piece
		return for file in
			getFiles (base + "*.max")
			collect file
	)
	
	fn getVariationCandidates = (
		getVariationCandidatesOf (pieceList.selected)
	)
	
	fn getCurrentPieceFileName = (
		return (getVariationCandidates())[variationSPN.value]
	)
	
	fn createPiece = (
		local piece = foundPieces.getItem (pieceList.selected)
		newVINE_Piece piece.kit piece.piece (getFileNameFile(getCurrentPieceFileName()))
	)
	
	fn loadBitMap = (
		local name = getCurrentPieceFileName()
		local path = getFileNamePath name
		local file = getFileNameFile name
		local icon = path + "\\" + file + ".png"
		if doesFileExist icon
		then pieceIconBMP.fileName = icon
		else pieceIconBMP.bitmap = BitMap 128 128 color:(color 23 23 23) ()
	)
	
	fn initVariations sel = (
		if sel != -1 and pieceList.items.count > 0 then (
			local piece = foundPieces.getItem (pieceList.items[sel])
			variationSPN.range = [1, piece.variations, 1]
			variationSPN.value = amin#(random 1 (piece.variations + 1), piece.variations)
		)
	)
	
	fn populateDictionaryFromKit kitName = (
		local kits_path = VINE_meshesPath+"kits\\"
		local maxFilePieces = #()
		for file in getFiles (kits_path + kitName+"\\*.max") do (
			local piece = pieceName()
			piece.kit = kitName
			local noVarName
			local b = true
			piece.piece	= (filterString (getFilenameFile file) "-")[1]
			for a in maxFilePieces do
				if (stricmp a.piece piece.piece)==0 then (
					a.variations += 1
					b = false
				)
				
			if b then  append maxFilePieces piece
		)
		for a in maxfilePieces do foundPieces.addItem (a.asString()) a
	)
	
	fn filterPiece piece text = (
		local l = matchPattern (piece.asString()) pattern:("*"+text+"*")
		l = l and (kitSelDDL.selection == 1 or (stricmp piece.kit kitSelDDL.text)==0)
		return l 
	)
	
	fn filterPieces text = (
		pieceList.items =
			for piece in foundPieces.items
			where filterPiece piece text
			collect piece.asString()
		pieceList.selection = 1
		initVariations 1
		loadBitMap()
	)
	
	button clearFilter "X" pos:[336,32] width:24 height:16 toolTip:"Remove the name filter."
	
	button toggleCollisionBTN "Toggle Collision" pos:[264,368] width:88 height:24 toolTip:"Toggle collision on selected pieces"
	button randRotXBTN "X" pos:[152,416] width:16 height:16 toolTip:"Rotate randomly in right angle increments along the X axis."
	
	fn selectClass class = (
		selSet = for a in $* where classOf a == class collect a
		if selSet.count == 0 then clearSelection() else select selSet
	)
		
	button randRotYBTN "Y" pos:[176,416] width:16 height:16 toolTip:"Rotate randomly in right angle increments along the Y axis."
	button randRotZBTN "Z" pos:[200,416] width:16 height:16 toolTip:"Rotate randomly in right angle increments along the Z axis."
	button cleanupBTN "Normal" pos:[192,472] width:48 height:24 enabled:true toolTip:"Clean up, update meaningful wireframe colors, etc."
	
	fn ensureAABB obj = (
		if obj.rotation != quat 0 0 0 1 then (
			format "Object of class % are AABBs! They must be aligned!\n" (classof obj)
			removeRotation obj
		)
	)
		
	fn cleanUpScene = (
		VINE_LevelGarbageCollection()
		local objPostDelete = #()
		for obj in $* do (
			if obj.name == "COLLISION" then (
				obj.material = VINE_collisionMaterial
				obj.wirecolor = VINE_collisionMaterial.selfIllumColor
				obj.isHidden = VINE_collisionToggle
			)
			case classof obj of (
				VINE_Piece: (
						if isTransformablePiece obj
							then obj.thisObject.wirecolor = color 0 255 0
						if isSpecialPiece obj
							then obj.thisObject.wirecolor = color 196 0 127
						if not isGone obj.collisionMesh \
						and not matchpattern obj.collisionMesh.name pattern:"COLLISION*"
							then obj.collisionMesh.name = "COLLISION"
					)
				VINE_Creep: (
						if obj.delegate.masterMesh == undefined then append objPostDelete obj
						else if not isGone obj.parent and classof obj.parent == PLANE
							then (
								if obj.parent.material == undefined or classof obj.parent.material != StandardMaterial
									then obj.parent.material = StandardMaterial()
								obj.parent.material.diffuse = obj.wirecolor
								obj.parent.wirecolor = obj.wirecolor
							)
							else format "Creep % still missing a Plane parent!\n" obj.name
					)
				VINE_CheckPoint: if obj.delegate.masterMesh == undefined then append objPostDelete obj
				VINE_Liana: if obj.delegate.masterMesh == undefined then append objPostDelete obj
				VINE_Trampoline: if obj.delegate.masterMesh == undefined then append objPostDelete obj
				VINE_Cannon: if obj.delegate.masterMesh == undefined then append objPostDelete obj
				VINE_Pickup: if obj.delegate.masterMesh == undefined then append objPostDelete obj
				VINE_Trigger: (ensureAABB obj; obj.material = VINE_triggerMaterial)
				VINE_CheckPointTrigger: (ensureAABB obj; obj.material = VINE_CheckPointMaterial)
				VINE_SpatialIndex: (ensureAABB obj; obj.material = VINE_SpatialIndexMaterial)
				VINE_Destructible: (obj.material = VINE_DestructibleMaterial)
			)
		)
		if objPostDelete.count > 0 then (
			format "Deleted % corrupted VINE_Objects.\n" objPostDelete.count
			for obj in objPostDelete do delete obj
		)
	)
	
	
	button reloadBTN "Reload" pos:[152,368] width:48 height:24 toolTip:"Reload selected objects."
	button expPiecesButton "All" pos:[312,504] width:40 height:24 enabled:true toolTip:"Export all available pieces."
	button seeCollisionBTN "Static only" pos:[104,472] width:64 height:24 enabled:true toolTip:"Generate the level mesh that will be exported along the level."
	GroupBox grp2 "Export level" pos:[8,456] width:80 height:80
	label randRotLBL "Rotate randomly" pos:[152,400] width:88 height:16 
 
	button rotate4BTN "3" pos:[224,416] width:16 height:16 toolTip:"Rotate randomly in all axes"
	
	fn rotatePieces axis ang:undefined = (
		if ang==undefined then ang = (((random 1 4)as integer)*90) 
		local selSet = #()
		if $ != undefined
		then selSet = $
		if selSet.count != 0 then (
			for obj in selSet where isgrouphead obj do (
				rotate obj (angleaxis ang axis)
			)
		)
	)
	button cleanUpPlusBTN "Extra" pos:[192,504] width:48 height:24 enabled:true toolTip:"Do a cleanup and delete all pattern meshes."
	
	button replaceBTN "Replace" pos:[208,368] width:48 height:24 enabled:true toolTip:"Replace all selected pieces with the selection from the list."
	GroupBox onSelectedGRP "On selected objects" pos:[144,352] width:216 height:88
	button seeCollisionPlusBTN "Everything" pos:[104,504] width:64 height:24 enabled:true toolTip:"Generate a collision mesh that includes the transformable pieces."
	
	fn checkCollision transformables = (
		local exporter = VINE_exportLevel()
		local emesh = Editable_Mesh ()
		local colMesh = exporter.allCollisions transformables:transformables ()
		emesh.mesh= colMesh
		if meshop.getNumVerts emesh.mesh > 0 then (
			emesh.name = VINE_JoinedColName
			emesh.material = VINE_collisionMaterial2
			emesh.wirecolor = VINE_collisionMaterial2.selfIllumColor
			select emesh
			local msg = "Remember to delete this object!"
			format "%\n" msg
			if VINE_QUIET == undefined then messagebox msg title:(emesh.name)
		) else (
			delete emesh
			delete colMesh
			format "No collision!\n"
		)
	)

	button quickExportLevelBTN "Quick" pos:[16,504] width:64 height:24 enabled:true toolTip:"Export the level xml only, without doing any extra job. (Use this only if the collision hasn't changed, for example, when moving objects around)"
	GroupBox grp3 "See collision" pos:[96,456] width:80 height:80
	GroupBox grp4 "Cleanup" pos:[184,456] width:64 height:80
	
	fn exportPiece kitName pieceName exporter:undefined = (
		local piecepath = (VINE_meshesPath+ \
			"kits\\"+kitName+"\\"+pieceName+".max")
							
		if exporter==undefined then exporter = VINE_meshExporter()
			
		local obj = undefined			
		local isOK = mergemaxFile piecepath \
			#("PIECE") \
			#select #noRedraw \
			#mergeDups #useMergedMtlDups 
			
		if not isOk then (
			format "Failed to load %kits/%.max\n%" VINE_meshesPath className \
				"Did you forget to set the proper path in the options?"
			return false
		) else (
			obj = $
		)
		
		if obj != undefined then (
			local groupHead = false
			try (
				groupHead = isgrouphead obj
			) catch (
				format "isGroupHead failed: %\n" (getCurrentException())
			) 
			if groupHead then (
				format "Piece %-% wasn't exported %\n" \
					kitName pieceName "(group exporting not supported yet)"
				setGroupOpen obj false
				delete obj
			) else (
				obj.name = "PIECE-"+kitName+"-"+pieceName
				format "Exporting %\n" obj.name
				exporter.exportStatic obj
				exporter.exportMaterials obj
				format "\n"
				delete obj
			)
		)
	)
	
	fn exportKit kitName exporter:undefined = (
		local exporter = VINE_meshExporter()
		-- For each piece
		for file in getFiles (VINE_meshesPath+"kits\\"+ kitName+"\\*.max") do (
			exportPiece kitName (getFilenameFile file) exporter:exporter
		)
	)
	
	
	
	button exportFilteredBTN "Filter" pos:[264,504] width:40 height:24 enabled:true toolTip:"Export all available pieces."
	button exportKitBTN "Kit" pos:[312,472] width:40 height:24 enabled:true toolTip:"Export all available pieces."
	button exportSinglePieceBTN "Piece" pos:[264,472] width:40 height:24 enabled:true toolTip:"Export a single piece"
	GroupBox grp9 "Export pieces" pos:[256,456] width:104 height:80
	on ROLLOUT_VINE_kits open do
	(
		foundPieces = Dictionary()
		local kits_path = VINE_meshesPath+"kits\\"
		local kits = getDirectories (kits_path + "*")
		for a in kits do (
			local kitName = getFilenameDir a
			kitSelDDL.items = append kitSelDDL.items kitName
			populateDictionaryFromKit kitName
		)
		if foundPieces.items.count > 0 then (
			pieceList.items = foundPieces.names
			pieceList.selection = 1
			initVariations 1
			loadBitMap()
		) else (
			kitSelDDL.enabled = false
			filterTxtIn.enabled = false
			clearFilter.enabled = false
			pieceList.enabled = false
			pieceList.items = #("No pieces have been found. Revise the options.")
			variationSPN.enabled = false
			newBTN.enabled = false
			exportSinglePieceBTN.enabled = false
			exportFilteredBTN.enabled = false
			exportKitBTN.enabled = false
			findBTN.enabled = false
			pieceIconBMP.enabled = false
		)
	)
	on kitSelDDL selected sel do
	(
			filterPieces (filterTxtIn.text)
			pieceList.selection = 1
			loadBitMap()
		)
	on pieceList selected sel do
	(
		VINE_LevelGarbageCollection()
		initVariations sel
		loadBitMap()
	)
	on pieceList doubleClicked sel do
	(
		createPiece()
	)
	on filterTxtIn entered text do
		filterPieces text
	on newBTN pressed do
	(
		if pieceList.selection != 0
		then createPiece ()
	)
	on findBTN pressed do
	(
		if pieceList.selection != 0 then (
			local piece = foundPieces.getItem (pieceList.selected)
			select (
				for obj in $*
				where (
					classof obj == VINE_Piece and
					(stricmp  obj.pieceKit piece.kit)==0 and
					(stricmp  obj.pieceName piece.piece)==0
					)
				collect obj.parent
			)
			--format "FIND: selecting all %-%.\n" piece.kit piece.piece
		) else (
			--format "FIND: no object selected.\n"
		)
	)
	on randBTN pressed do
	(
		local selSet = #()
		if $ != undefined
		then (
			for a in $ do (
				for b in a where classOf b == VINE_Piece do (
					append selSet b
				)
			)
		)
		if selSet.count != 0 then (
			for obj in selSet do (
				local kit = obj.pieceKit
				local name = obj.pieceName
				local str = undefined
				for entry in pieceList.items do (
					if  matchPattern entry pattern:("["+kit+"]"+name+" (*)") then (
						str = entry
						break
					)
				)
				if str != undefined then (
					local piece = foundPieces.getItem str
					local vars = piece.variations
					local r = amin #(random 1 (vars+1), vars)
					local fileName = (getVariationCandidatesOf str)[r]
					local newMesh = obj.changeTo kit name (getFileNameFile fileName) sel:false
					obj.parent = newMesh
					selectMore newMesh
				)
			)
		)
		if pieceList.selection != 0 then (
			local r = variationSPN.range
			variationSPN.value = amin #(random 1 (r[2]+1), r[2])
			loadBitMap()
		)
	)
	on variationSPN changed val do
		loadBitMap()
	on levelExpBTN pressed do
	(
		cleanUpScene()
		local exporter = VINE_exportLevel()
		local name = getFilenameFile maxFileName
		if name=="" then name = "untitled"
		exporter.exportScene name
	)
	on objectSelDDL selected sel do
	(
		local selSet
		case objectSelDDL.items[sel] of (
			"Nothing": clearSelection()
			"Pieces": selectClass VINE_Piece
			"Melees": selectClass VINE_Melee
			"Flares": selectClass VINE_Flare
			"Snipers": selectClass VINE_Sniper
			"Trampolines": selectClass VINE_Trampoline
			"Cannons": selectClass VINE_Cannon
			"Creeps": selectClass VINE_Creep
			"Lianas": selectClass VINE_Liana
			"CheckPoints": selectClass VINE_CheckPoint
		)
	)
	on clearFilter pressed do
	(
			filterTxtIn.text = ""
			filterPieces (filterTxtIn.text)
		)
	on toggleCollisionBTN pressed do
	(
		local selSet = #()
		if $ != undefined
		then selSet = deepGroup $
		if selSet.count != 0 then (
			for obj in selSet do (
				case obj.name of (
					"COLLISION": (
						obj.name = "COLLISION[DISABLED]"
						obj.isHidden = true;
					)
					"COLLISION[DISABLED]": (
						obj.name = "COLLISION"
						obj.isHidden = VINE_collisionToggle
					)
				)
			)
		)
	)
	on randRotXBTN pressed do
		rotatePieces [1,0,0]
	on randRotYBTN pressed do
		rotatePieces [0,1,0]
	on randRotZBTN pressed do
		rotatePieces [0,0,1]
	on cleanupBTN pressed do
	(
			local d = for o in $* where o.name == VINE_JoinedColName collect o
			for o in d do delete o
			cleanUpScene()
		)
	on reloadBTN pressed do
	(
		local selSet = #()
		if $ != undefined
		then (
			selSet =
				for a in deepgroup $
					where classOf a == VINE_Piece
					collect a
		)
		if selSet.count != 0 then (
			local reloaded = #()
			for piece in selSet do (
				local kit = piece.pieceKit
				local name = piece.pieceName
				local filename = piece.pieceFileName
				local reload = (findItem  reloaded (kit +"/"+ filename)) == 0
				if reload then append reloaded (kit +"/"+ filename)
				local newMesh = piece.changeTo kit name fileName sel:false reloadMaster:reload
				piece.parent = newMesh
				selectMore newMesh
			)
		)
	)
	on expPiecesButton pressed do
	(
		with undo off (
			with redraw off (
				local exporter = VINE_meshExporter()
				local kits = getDirectories (VINE_meshesPath+"kits\\" + "*")
				-- For each kit
				for a in kits do (
					exportKit (getFilenameDir a) exporter:exporter
				)
			)
		)
	)
	on seeCollisionBTN pressed do
		checkCollision false
	on rotate4BTN pressed do
	(
			rotatePieces [1,0,0] ang:(random 1 360)
			rotatePieces [0,1,0] ang:(random 1 360)
			rotatePieces [0,0,1] ang:(random 1 360)
		)
	on cleanUpPlusBTN pressed do
	(
	   cleanUpScene()
		
		VINE_pieceDictionary = Dictionary()
		deleteLayer "MASTER_PIECES"
		OK
	)
	on replaceBTN pressed do
	(
		local selSet = #()
		if $ != undefined
		then (
			selSet =
				for a in deepgroup $
					where classOf a == VINE_Piece
					collect a
		)
		if selSet.count != 0 then (
			local reload = true
			local listPiece = foundPieces.getItem (pieceList.selected)
			local kit = listPiece.kit
			local name = listPiece.piece
			local filename = getFileNameFile(getCurrentPieceFileName())
			for piece in selSet do (
				local newMesh = piece.changeTo kit name fileName sel:false reloadClass:reload
				reload = false
				piece.parent = newMesh
				selectMore newMesh
			)
		)
	)
	on seeCollisionPlusBTN pressed do
		checkCollision true
	on quickExportLevelBTN pressed do
	(
		cleanUpScene()
		local exporter = VINE_exportLevel()
		local name = getFilenameFile maxFileName
		if name=="" then name = "untitled"
		exporter.exportScene name quick:true
	)
	on exportFilteredBTN pressed do
	(
		with undo off (
			with redraw off (
				if filterTxtIn.text == "" then (
					messagebox "Enter a filter first or, if you want to export all pieces, hit the \"All\" button.\n" title:"Hey!"
				) else (
					local exporter = VINE_meshExporter()
					for item in pieceList.items do (
						local listPiece = foundPieces.getItem item
						exportPiece listPiece.kit listPiece.piece exporter:exporter
					)
				)
			)
		)
	)
	on exportKitBTN pressed do
	(
		with undo off (
			with redraw off (
				if kitSelDDL.selection==1
					then messagebox "Select a kit first, or, if you want to export all pieces, hit the \"All\" button." title:"Hey!"
					else exportKit kitSelDDL.selected exporter:VINE_meshExporter()
			)
		)
	)
	on exportSinglePieceBTN pressed do
	(
		with undo off (
			with redraw off (
				local piece = foundPieces.getItem (pieceList.selected)
				exportPiece piece.kit piece.piece
			)
		)
	)
)

fn LOAD_MACRO_VINE_kits = (
	macroScript MACRO_VINE_kits
		category:"VINEDETTA"
		toolTip: "Kits"
		icon:#("kits", 1)
	(
		addrollout ROLLOUT_VINE_kits \
			(newRolloutFloater "VINE level editor piece picker" 376 600) \
			border:false
	)
	OK
)
LOAD_MACRO_VINE_kits ()
