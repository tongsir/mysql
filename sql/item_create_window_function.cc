/* Copyright (C) 2013 Calpont Corp.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

/***********************************************************************
*   $Id: item_create_window_function.cc 9559 2013-05-22 17:58:13Z xlou $
*
*
***********************************************************************/

/**
  @file

  @brief
  Functions to create a window function item. Used by sql_yacc.yy
*/

#include "sql_priv.h"
#include "item_create_window_function.h"
#include "item_window_function.h"
#include "sp_head.h"
#include "sp.h"

/*
=============================================================================
  LOCAL DECLARATIONS
=============================================================================
*/

/**
  Adapter for functions that takes exactly zero arguments.
*/

Create_window_func_arg0 Create_window_func_arg0::s_singleton;

/**
  Adapter for functions that takes exactly one argument.
*/

Create_window_func_arg1 Create_window_func_arg1::s_singleton;

/**
  Adapter for functions that takes exactly two arguments.
*/

Create_window_func_arg2 Create_window_func_arg2::s_singleton;

/**
  Adapter for functions that takes exactly three arguments.
*/

Create_window_func_arg3 Create_window_func_arg3::s_singleton;

// rank functions
class Create_window_func_rank : public Create_window_func_arg0
{

public:
	virtual Item *create(THD *thd, LEX_STRING name)
	{
	  return new (thd->mem_root) Item_func_window_rank(name);
	}

	virtual Item *create(THD *thd, LEX_STRING name, List<Item> *item_list)
	{
		return create(thd, name);
	}

	static Create_window_func_rank s_singleton;

protected:
	Create_window_func_rank() {}
	virtual ~Create_window_func_rank() {}
};

Create_window_func_rank Create_window_func_rank::s_singleton;

// row_number function
class Create_window_func_rownumber : public Create_window_func_arg0
{
public:
	virtual Item *create(THD *thd, LEX_STRING name)
	{
	  return new (thd->mem_root) Item_func_window_rownumber(name);
	}

	virtual Item *create(THD *thd, LEX_STRING name, List<Item> *item_list)
	{
		return create(thd, name);
	}

	static Create_window_func_rownumber s_singleton;

protected:
	Create_window_func_rownumber() {}
	virtual ~Create_window_func_rownumber() {}
};

Create_window_func_rownumber Create_window_func_rownumber::s_singleton;

// nth_value function
Item* Create_window_func_nth_value::create(THD *thd, LEX_STRING name, List<Item> *item_list)
{
	int arg_count= 0;

	if (item_list)
	  arg_count= item_list->elements;

	if (arg_count < 1)
	{
		IDB_set_error(thd, logging::ERR_WF_WRONG_ARGS, name.str);
		return NULL;
	}
	
	if (arg_count == 1 && strcasecmp(name.str, "NTH_VALUE") == 0)
	{
		IDB_set_error(thd, logging::ERR_WF_WRONG_ARGS, name.str);
		return NULL;
	}
	
	if (arg_count > 1 && strcasecmp(name.str, "NTH_VALUE") != 0)
	{
		IDB_set_error(thd, logging::ERR_WF_WRONG_ARGS, name.str);
		return NULL;
	}

	Item *param1 = NULL, *param2 = NULL;
	if (arg_count >= 1)
		param1= item_list->pop();
	if (arg_count >= 2)
		param2 = item_list->pop();

	// offset default to be 1 for first_value and last_value
	if (!param2)
		param2 = new (thd->mem_root) Item_int(1);
	
	// add fromFirst and respectNulls to the arg list
	if (strcasecmp(name.str, "FIRST_VALUE") == 0)
		fromFirst = 1;
	else if (strcasecmp(name.str, "LAST_VALUE") == 0)
		fromFirst = 0;
	Item *param3 = new (thd->mem_root) Item_int((int)fromFirst);
	Item *param4 = new (thd->mem_root) Item_int((int)respectNulls);

	return create(thd, name, param1, param2, param3, param4);
}
	
Item* Create_window_func_nth_value::create(THD *thd, 
	                                       LEX_STRING name, 
	                                       Item *arg1, 
	                                       Item *arg2,
	                                       Item *arg3,
	                                       Item *arg4)
{
	return new (thd->mem_root) Item_func_window_nth_value(name, arg1, arg2, arg3, arg4);
};

Create_window_func_nth_value Create_window_func_nth_value::s_singleton;

// lead/lag function
class Create_window_func_lead_lag : public Create_window_func
{
public:
	virtual Item *create(THD *thd, LEX_STRING name, List<Item> *item_list)
	{
		int arg_count= 0;

		if (item_list)
		  arg_count = item_list->elements;

		if (arg_count < 1)
		{
			IDB_set_error(thd, logging::ERR_WF_WRONG_ARGS, name.str);
			return NULL;
		}

		Item *param1 = NULL, *param2= NULL, *param3= NULL;
		if (arg_count >= 1)
			param1 = item_list->pop();
		if (arg_count >= 2)
			param2 = item_list->pop();
		if (arg_count == 3)
			param3 = item_list->pop();

		// offset default to be 1
		if (!param2)
			param2 = new (thd->mem_root) Item_int(1);

		// default value default to be null
		if (!param3)
			param3 = new (thd->mem_root) Item_null();
		
		Item *param4 = new (thd->mem_root) Item_int(int(respectNulls));

		return create(thd, name, param1, param2, param3, param4);
	}

	virtual Item *create(THD *thd, LEX_STRING name, Item *arg1, Item *arg2, Item *arg3, Item* arg4)
	{
		return new (thd->mem_root) Item_func_window_lead_lag(name, arg1, arg2, arg3, arg4);
	}

	static Create_window_func_lead_lag s_singleton;

protected:
	Create_window_func_lead_lag() {}
	virtual ~Create_window_func_lead_lag() {}
};

Create_window_func_lead_lag Create_window_func_lead_lag::s_singleton;

// median function
class Create_window_func_median : public Create_window_func_arg1
{
public:
	virtual Item *create(THD *thd, LEX_STRING name, Item *arg1)
	{
		return new (thd->mem_root) Item_func_window_median(name, arg1);
	}

	virtual Item *create(THD *thd, LEX_STRING name, List<Item> *item_list)
	{
		return create(thd, name, item_list->pop());
	}

	static Create_window_func_median s_singleton;

protected:
	Create_window_func_median() {}
	virtual ~Create_window_func_median() {}
};

Create_window_func_median Create_window_func_median::s_singleton;

// ntile function
class Create_window_func_ntile : public Create_window_func_arg1
{
public:
	virtual Item *create(THD *thd, LEX_STRING name, Item *arg1)
	{
		return new (thd->mem_root) Item_func_window_ntile(name, arg1);
	}

	virtual Item *create(THD *thd, LEX_STRING name, List<Item> *item_list)
	{
		return create(thd, name, item_list->pop());
	}

	static Create_window_func_ntile s_singleton;

protected:
	Create_window_func_ntile() {}
	virtual ~Create_window_func_ntile() {}
};

Create_window_func_ntile Create_window_func_ntile::s_singleton;

struct Native_window_func_registry
{
	LEX_STRING name;
	Create_window_func *builder;
};

#define BUILDER(F) & F::s_singleton

/*
  InfiniDB window functions.
  MAINTAINER:
  - Keep sorted for human lookup. At runtime, a hash table is used.
  - do **NOT** conditionally (#ifdef, #ifndef) define a function *NAME*:
    doing so will cause user code that works against a --without-XYZ binary
    to fail with name collisions against a --with-XYZ binary.
    Use something similar to GEOM_BUILDER instead.
  - keep 1 line per entry, it makes grep | sort easier
*/

// Window functions with common syntax
static Native_window_func_registry func_array[] =
{
	{ { C_STRING_WITH_LEN("CUME_DIST") }, BUILDER(Create_window_func_rank)},
	{ { C_STRING_WITH_LEN("DENSE_RANK") }, BUILDER(Create_window_func_rank)},
	{ { C_STRING_WITH_LEN("FIRST_VALUE") }, BUILDER(Create_window_func_nth_value)},
	{ { C_STRING_WITH_LEN("LAST_VALUE") }, BUILDER(Create_window_func_nth_value)},
	{ { C_STRING_WITH_LEN("LAG") }, BUILDER(Create_window_func_lead_lag)},
	{ { C_STRING_WITH_LEN("LEAD") }, BUILDER(Create_window_func_lead_lag)},
	{ { C_STRING_WITH_LEN("MEDIAN") }, BUILDER(Create_window_func_median)},
	{ { C_STRING_WITH_LEN("NTILE") }, BUILDER(Create_window_func_ntile)},
	{ { C_STRING_WITH_LEN("PERCENT_RANK") }, BUILDER(Create_window_func_rank)},
	{ { C_STRING_WITH_LEN("RANK") }, BUILDER(Create_window_func_rank)},
	{ { C_STRING_WITH_LEN("ROW_NUMBER") }, BUILDER(Create_window_func_rownumber)},
	{ {0, 0}, NULL}
};

// Window functions with RESPECT|IGNORE NULLS
static Native_window_func_registry func_array_nulls[] =
{
	{ { C_STRING_WITH_LEN("FIRST_VALUE") }, BUILDER(Create_window_func_nth_value)},
	{ { C_STRING_WITH_LEN("LAST_VALUE") }, BUILDER(Create_window_func_nth_value)},
	{ { C_STRING_WITH_LEN("LAG") }, BUILDER(Create_window_func_lead_lag)},
	{ { C_STRING_WITH_LEN("LEAD") }, BUILDER(Create_window_func_lead_lag)},
	{ { C_STRING_WITH_LEN("NTH_VALUE") }, BUILDER(Create_window_func_nth_value)},
	{ {0, 0}, NULL}
};

static HASH native_window_functions_hash;
static HASH native_window_functions_hash_nulls;

extern "C" uchar*
get_native_window_fct_hash_key(const uchar *buff, size_t *length,
                        my_bool /* unused */)
{
	Native_window_func_registry *func= (Native_window_func_registry*) buff;
	*length= func->name.length;
	return (uchar*) func->name.str;
}

/*
  Load the hash table for window functions.
  Note: this code is not thread safe, and is intended to be used at server
  startup only (before going multi-threaded)
*/

int item_window_function_create_init()
{
	Native_window_func_registry *func;
	
	DBUG_ENTER("item_create_init");
	
	if (my_hash_init(& native_window_functions_hash,
	              system_charset_info,
	              array_elements(func_array),
	              0,
	              0,
	              (my_hash_get_key) get_native_window_fct_hash_key,
	              NULL,                          /* Nothing to free */
	              MYF(0)))
		DBUG_RETURN(1);
	
	for (func= func_array; func->builder != NULL; func++)
	{
		if (my_hash_insert(& native_window_functions_hash, (uchar*) func))
			DBUG_RETURN(1);
	}

#ifndef DBUG_OFF
	for (uint i=0 ; i < native_window_functions_hash.records ; i++)
	{
		func= (Native_window_func_registry*) my_hash_element(& native_window_functions_hash, i);
		DBUG_PRINT("info", ("native function: %s  length: %u",
		          func->name.str, (uint) func->name.length));
	}
#endif

	if (my_hash_init(& native_window_functions_hash_nulls,
	              system_charset_info,
	              array_elements(func_array_nulls),
	              0,
	              0,
	              (my_hash_get_key) get_native_window_fct_hash_key,
	              NULL,                          /* Nothing to free */
	              MYF(0)))
		DBUG_RETURN(1);
	
	for (func= func_array_nulls; func->builder != NULL; func++)
	{
		if (my_hash_insert(& native_window_functions_hash_nulls, (uchar*) func))
			DBUG_RETURN(1);
	}

#ifndef DBUG_OFF
	for (uint i=0 ; i < native_window_functions_hash_nulls.records ; i++)
	{
		func= (Native_window_func_registry*) my_hash_element(& native_window_functions_hash_nulls, i);
		DBUG_PRINT("info", ("native function: %s  length: %u",
		          func->name.str, (uint) func->name.length));
	}
#endif

  DBUG_RETURN(0);
}

/*
  Empty the hash table for window functions.
  Note: this code is not thread safe, and is intended to be used at server
  shutdown only (after thread requests have been executed).
*/

void item_window_function_create_cleanup()
{
	DBUG_ENTER("item_create_cleanup");
	my_hash_free(& native_window_functions_hash);
	my_hash_free(& native_window_functions_hash_nulls);
	DBUG_VOID_RETURN;
}

Create_window_func *
find_native_window_function_builder(THD *thd, LEX_STRING name)
{
	Native_window_func_registry *func;
	Create_window_func *builder= NULL;

	/* Thread safe */
	func= (Native_window_func_registry*) my_hash_search(& native_window_functions_hash,
	                                                 (uchar*) name.str,
	                                                 name.length);

	if (func)
	{
		builder= func->builder;
	}

	return builder;
}

Create_window_func *
find_native_window_function_builder_nulls(THD *thd, LEX_STRING name)
{
	Native_window_func_registry *func;
	Create_window_func *builder= NULL;

	/* Thread safe */
	func= (Native_window_func_registry*) my_hash_search(& native_window_functions_hash_nulls,
	                                                 (uchar*) name.str,
	                                                  name.length);

	if (func)
	{
		builder= func->builder;
	}

	return builder;
}

