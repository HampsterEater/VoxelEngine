// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef STATE_SETTING
#error "STATE_SETTING must be defined when including this file!"
#endif

// These define all the different state-enum-values that can be configured in the shader
// pipeline xml file.

//			  Name					Enum			

// Samplers	
STATE_SETTING("LOWER_OR_EQUAL",		Lower_Or_Equal)
STATE_SETTING("LOWER",				Lower)
STATE_SETTING("GREATER_OR_EQUAL",	Greater_Or_Equal)
STATE_SETTING("GREATER",			Greater)
STATE_SETTING("EQUAL",				Equal)
STATE_SETTING("NOT_EQUAL",			Not_Equal)
STATE_SETTING("NEVER",				Never)
STATE_SETTING("ALWAYS",				Always)

STATE_SETTING("BACK",				Back)
STATE_SETTING("FRONT",				Front)

STATE_SETTING("ONE_ONE",						One_One)
STATE_SETTING("SRC_ALPHA_ONE_MINUS_SRC_ALPHA",	Src_Alpha_One_Minus_Src_Alpha) // TODO: gross