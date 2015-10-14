global max2mcv_m =  RotateXMatrix -90
global mcv2max_m = inverse(max2mcv_m)

fn max2mcv v = (
	return v * max2mcv_m
)

fn max2mcvMat m = (
	return mcv2max_m * m * max2mcv_m;
)

fn max2mcvQuat q = (
	return (max2mcvMat (q as Matrix3)) as quat
)

fn minmax &a &b =
(
	local af = copy a
	local bf = copy b
	--Guarantee forall i A.i < B.i
	if (b.x < a.x) then (
		af.x = b.x
		bf.x = a.x
		)
	if (b.y < a.y) then (
		af.y = b.y
		bf.y = a.y
		)
	if (b.z < a.z) then (
		af.z = b.z
		bf.z = a.z
		)
	a = af
	b = bf
	OK
)

fn ensurePathExists dirName = (
	if not doesDirectoryExist dirName then (
		makeDir dirName
	)
	OK
)

fn cat filename = (
	local file = openFile filename mode:"r"
	if file != undefined then (
		while not eof file do (
			format "%\n" (readLine file)
		) 
		close file
		local _ = gc()
	)
)

fn verticesOf obj = (
	local ans = #()
	for i = 1 to getNumVerts obj do (
		append ans (getVert obj i)
	)
	return ans
)

fn facesOf obj = (
	local ans = #()
	for i = 1 to getNumFaces obj do (
		append ans (getFace obj i)
	)
	return ans
)

fn printLongArray a = (
	format "#("
	for i in a do format "%" i
	format ")"
)

fn reverseMesh &a = (
	local nfa = meshop.getnumfaces a
	for i = 1 to nfa do (
		face = getface a i
		local i2 = face[2]
		face[2] = face[3]
		face[3] = i2
		setface a i face
	)
	return a
)

struct Dictionary (
	nItems = 0,
	items = #(),
	names = #(),
	
	fn getItemIndex nam = (
		for i=1 to nItems do (
			if names[i] == nam then return i
			)
		return 0
	),
	
	fn getItem nam = (
		local i = getItemIndex nam
		if  i == 0
		then return undefined
		else return items[i]
	),
	
	fn addItem nam item = (
		if getItemIndex nam == 0
		then (
			nItems = nItems + 1
			append items item
			append names nam
			return nItems
		) else return i
	),
	
	fn hasItem nam = return getItemIndex nam != 0,
		
	fn eraseItem nam = (
		local i = getItemIndex nam
		if i > 0 then (
			local item = items[i]
			deleteItem items i
			deleteItem names i
			return item
		) else return undefined
	)
)

fn getFilenameDir dir = (
	local chain = filterString dir "\\"
	return chain[chain.count]
)

fn round f nDecimals  = (
	local tens = 10^nDecimals
	return (((f*tens) as integer)as float)/tens
)

fn deepGroup_aux oSet = (
	local curr = #()
	for a in oSet do (
		if classOf a == ObjectSet
			then join curr (deepGroup_aux a)
			else append curr a
	)
	return curr
)

fn deepGroup oSet = (
	if classof oSet == ObjectSet
		then return deepGroup_aux oSet
		else return #()
)

fn isGone obj = (return obj == undefined or isDeleted obj)

fn deleteLayer layerName = (
	local layer = LayerManager.getLayerFromName layerName
	if layer != undefined then (
		layer.isFrozen = false
		local nodes = #()
		if layer.nodes &nodes then
			for obj in nodes do 
				if not isDeleted obj then delete obj
		if layer.current then
			for i=0 to LayerManager.count-1 do (
				local otherLayer = LayerManager.getLayer i
				if not otherLayer.current then (
					otherLayer.current = true
					break;
				)
			)
		LayerManager.deleteLayerByName layerName
	)
)

fn isSpecialPiece obj = (
	if obj.collisionMesh == undefined
		then return false
		else if obj.collisionMesh.name != "COLLISION" then return false
	if getUserProp obj "special2" != undefined then return true
	if getUserProp obj "special" != undefined then return true
	local propsPath = VINE_meshesPath+"kits\\"+obj.piecekit+ \
		"\\"+obj.piecefilename+".properties"
	if doesFileExist propsPath then (
		local file = openFile propsPath mode:"r"
		if file != undefined then (
			local ret = false
			while not eof file do (
				local l = readLine file
				if matchpattern l pattern:"*special=\"*\"*"
					then ret = true
			)
			close file
			gc()
			return ret
		)
	)
	return false
)

fn isTransformablePiece obj = (
	if obj.collisionMesh == undefined
		then return false
		else if obj.collisionMesh.name != "COLLISION" then return false
	if getUserProp obj "transformation2" != undefined then return true
	if getUserProp obj "transformation" != undefined then return true
	local propsPath = VINE_meshesPath+"kits\\"+obj.piecekit+ \
		"\\"+obj.piecefilename+".properties"
	if doesFileExist propsPath then (
		local file = openFile propsPath mode:"r"
		if file != undefined then (
			local ret = false
			while not eof file do (
				local l = readLine file
				if matchpattern l pattern:"*transformation=\"*\"*"
					then ret = true
			)
			close file
			gc()
			return ret
		)
	)
	return false
)

fn deepDir_aux dir pat &prev = (
	join prev (getFiles (dir + pat))
	for subdir in getDirectories (dir + "*") do
		deepDir_aux subdir pat &prev
	return prev
)

fn deepDir dir pat = (
	local ans = #()
	ans = deepDir_aux dir pat &ans
	return ans
)

fn removeRotation obj = (
	local pos = obj.position
	obj.rotation = quat 0 0 0 1
	obj.position = pos
	OK
)



