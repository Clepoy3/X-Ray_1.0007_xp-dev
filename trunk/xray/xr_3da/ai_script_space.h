//KRodin: ¬ этом файле подключаетс€ Luabind. “олько здесь и больше нигде.
#pragma once

#pragma comment( lib, "luabind.lib"	)

#pragma warning(push)

#pragma warning(disable:4244) // аргумент: преобразование "..." в "...", возможна потер€ данных
#pragma warning(disable:4995) // "...": им€ помечено как #pragma deprecated
#pragma warning(disable:4100) // unreferenced formal parameter
#pragma warning(disable:4127) // conditional expression is constant
#pragma warning(disable:4456) // declaration of 'x' hides previous local declaration
#pragma warning(disable:4458) // declaration of 'x' hides class member
#pragma warning(disable:4459) // declaration of 'x' hides global declaration
#pragma warning(disable:4913) // user defined binary operator 'x' exists but no overload could convert all operands
#pragma warning(disable:4297) // function assumed not to throw exception but does
#pragma warning(disable:4275) // class "...", не €вл€ющийс€ dll-интерфейсом, использован в качестве базового дл€ "..."
#pragma warning(disable:4389) // несоответствие типов со знаком и без знака
#pragma warning(disable:4101) // "...": неиспользованна€ локальна€ переменна€
#pragma warning(disable:4702) // Ќедостижимый код

extern "C" {
# include <lualib.h>
}

#include <luabind/luabind.hpp>
#include <luabind/class.hpp>
#include <luabind/object.hpp>
#include <luabind/operator.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/return_reference_to_policy.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/iterator_policy.hpp>
#include "xrScriptEngine/Functor.hpp"

#pragma warning(pop)
