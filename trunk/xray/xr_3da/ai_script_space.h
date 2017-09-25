//KRodin: В этом файле подключается Luabind. Только здесь и больше нигде.
#pragma once

#pragma comment( lib, "luabind.lib"	)

#define LUABIND_09

#ifdef LUABIND_09

#pragma warning(push)
#pragma warning(disable:4995 4297 4913 4275 4459)
#include <luabind/luabind.hpp>
#pragma warning(pop)

#include <luabind/class.hpp>
#include <luabind/object.hpp>
#include <luabindex/functor.hpp>
#include <luabind/operator.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/return_reference_to_policy.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/iterator_policy.hpp>

#else

#pragma warning(push)

#pragma warning(disable:4459) // declaration of 'x' hides global declaration
#pragma warning(disable:4913) // user defined binary operator 'x' exists but no overload could convert all operands
#pragma warning(disable:4297) // function assumed not to throw exception but does

#include <luabind/luabind.hpp>
#include <luabind/class.hpp>
#include <luabind/object.hpp>
#include <luabind/operator.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/return_reference_to_policy.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/iterator_policy.hpp>

#pragma warning(pop)

#endif
