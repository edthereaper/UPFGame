global VINE_version = "v0924a"

global TAG_mesh_begin 				= 0x33440000
global TAG_mesh_vertices  			= 0x33441111
global TAG_mesh_indices				= 0x33441122
global TAG_mesh_groups				= 0x33442211
global TAG_mesh_end   				= 0x3344FFFF

global TAG_level_enemy				= 0x335BAD1E
global TAG_level_class   			= 0x335C1A22
global TAG_level_melee   			= 0x335C0B2A
global TAG_level_flare   			= 0x335F1A2E
global TAG_level_sniper   			= 0x3352A1DA

global TAG_level_piece				= 0x336F1002
global TAG_level_object				= 0x3360B1EC
global TAG_level_trampoline			= 0x3360B001
global TAG_level_cannon				= 0x3360B002
global TAG_level_liana				= 0x3360B003
global TAG_level_creep				= 0x3360B004
global TAG_level_pickup				= 0x3360B005
global TAG_level_checkpoint			= 0x3360BE66

global TAG_level_trigger			= 0x3377BAC7
global TAG_level_checkpointTrigger  = 0x3377BE66
global TAG_level_spatialIndex		= 0x3377A2EA
global TAG_level_destructible		= 0x3377B28C
global TAG_level_whitebox			= 0x3377B0C2
global TAG_level_SmokeHazard		= 0x33779999

global TAG_level_wildcard			= 0x33880001
global TAG_level_WildcardBox		= 0x33880002

global COLOR_melee					= color 135 6 6
global COLOR_flare					= color 135 135 6
global COLOR_sniper					= color 6 6 135
global COLOR_gameObject				= color 6 135 135
global COLOR_gameObject_transformed	= color 6 135 6

global PRIMITIVE_points				= 4000
global PRIMITIVE_line_list			= 4001
global PRIMITIVE_triangle_list   	= 4002
global PRIMITIVE_triangle_strip 	= 4003

global VERTEX_pos					= 1001
global VERTEX_pos_uv				= 1002
global VERTEX_pos_color				= 1003
global VERTEX_pos_uv_normal			= 1004
global VERTEX_pos_uv_normal_tangent	= 1005
global VERTEX_skin			        = 1006

global VINE_settingsFile = (getDir #userScripts) + "\\..\\VINE_settings.ini"
global VINE_dataPath = getINISetting VINE_settingsFile "settings" "data_path"
global VINE_toolsPath = getINISetting VINE_settingsFile "settings" "tools_path"
global VINE_meshesPath = getINISetting VINE_settingsFile "Level Editor" "meshes_path"

global VINE_collisionMaterial = StandardMaterial ()
VINE_collisionMaterial.diffuse = red
VINE_collisionMaterial.selfillumination = 255
VINE_collisionMaterial.opacity = 1
VINE_collisionMaterial.useSelfIllumColor = on
VINE_collisionMaterial.selfIllumColor = red
VINE_collisionMaterial.name = "Collision shade"

global VINE_collisionMaterial2 = copy VINE_collisionMaterial
VINE_collisionMaterial2.diffuse = orange
VINE_collisionMaterial2.selfIllumColor = orange
VINE_collisionMaterial2.name = "Collision test shade"

global VINE_triggerMaterial = StandardMaterial ()
VINE_triggerMaterial.diffuse = blue
VINE_triggerMaterial.selfIllumColor = blue
VINE_triggerMaterial.useSelfIllumColor = on
VINE_triggerMaterial.opacity = 15
VINE_triggerMaterial.specularLevel = 60
VINE_triggerMaterial.name = "Trigger shade"

global VINE_CheckPointMaterial = copy VINE_triggerMaterial
VINE_CheckPointMaterial.diffuse = color 106 8 224
VINE_CheckPointMaterial.selfIllumColor = color 106 8 224
VINE_CheckPointMaterial.name = "CheckPoint shade"

global VINE_SpatialIndexMaterial = copy VINE_triggerMaterial
VINE_SpatialIndexMaterial.diffuse = gray 
VINE_SpatialIndexMaterial.opacity = 7
VINE_SpatialIndexMaterial.selfIllumColor = gray 
VINE_SpatialIndexMaterial.name = "Spatial Index Area shade"

global VINE_DestructibleMaterial = StandardMaterial ()
VINE_DestructibleMaterial.diffuse = color 0 255 255
VINE_DestructibleMaterial.opacity = 50
VINE_DestructibleMaterial.specularLevel = 30
VINE_DestructibleMaterial.name = "Destructible wall shade"

global VINE_WildCardBoxMaterial = copy VINE_triggerMaterial
VINE_WildCardBoxMaterial.diffuse = yellow
VINE_WildCardBoxMaterial.opacity = 7
VINE_WildCardBoxMaterial.selfIllumColor = yellow 
VINE_WildCardBoxMaterial.name = "Wilcard box shade"

global VINE_SmokeBoxMaterial = copy VINE_triggerMaterial
VINE_SmokeBoxMaterial.diffuse = (color 27 177 27)
VINE_SmokeBoxMaterial.opacity = 7
VINE_SmokeBoxMaterial.selfIllumColor = (color 27 177 27)
VINE_SmokeBoxMaterial.name = "Smoke box shade"
