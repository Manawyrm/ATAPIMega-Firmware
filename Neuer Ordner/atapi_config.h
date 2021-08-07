/*
 * Copyright (C) 2011 Tobias Himmer <provisorisch@online.de>
 *
 * This file is part of IDETrol.
 *
 * IDETrol is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * IDETrol is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with IDETrol.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * \defgroup ATAPI_CONFIG ATAPI-Interface Configuration
 * \brief ATAPI configuration.
 *
 * Configuration options for ATAPI layer.
 *
 * @{
 **/


#ifndef __ATAPI_CONFIG_H__INCLUDED__
#define __ATAPI_CONFIG_H__INCLUDED__


#ifndef ATAPI_USE_MALLOC
/// If this is enabled, functions with \e malloc support will compiled.
/**
 * For some functions, there are two versions. One which makes use of dynamic memory allocations using \e malloc etc, and another one that works without.
 * These function may differ slightly on how they are used.
 * They may both be compiled/used at the same time. However, to save space, you may choose to not compile one of the versions in.
 *
 * If you define this, functions that use \e malloc are built.
 **/
#define ATAPI_USE_MALLOC	1
#endif

#ifndef ATAPI_USE_NON_MALLOC
/// If this is enabled, functions that have \e malloc alternatives will be compiled.
/**
 * For some functions, there are two versions. One which makes use of dynamic memory allocations using \e malloc etc, and another one that works without.
 * These function may differ slightly on how they are used.
 * They may both be compiled/used at the same time. However, to save space, you may choose to not compile one of the versions in.
 *
 * If you define this, functions that don't use \e malloc but have alternate versions that use \e malloc, are built.
 **/
#define ATAPI_USE_NON_MALLOC	1
#endif

#ifndef ATAPI_USE_LBA
/// If this is enabled, \e LBA addressing support will be compiled.
/**
 * Use this to completely disable/enable \e LBA addressing support.
 **/
#define ATAPI_USE_LBA		1
#endif


#endif


/** @} **/
