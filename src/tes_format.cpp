#include <josk/tes_format.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <string_view>

namespace josk::tes
{

record_type_t to_record_type(const std::string_view record_type_string) noexcept
{
	if (record_type_string.size() != section_id_byte_size)
	{
		return record_type_t::none;
	}

	const auto itr = std::ranges::lower_bound(record_type_str, record_type_string);
	if (itr == record_type_str.cend())
	{
		return record_type_t::none;
	}

	return static_cast<record_type_t>(std::distance(record_type_str.cbegin(), itr));
}

}

namespace
{
using namespace josk::tes;

// record_type_str maps each valid record_type enum value with its TES char representation.
static_assert(record_type_str.size() == static_cast<std::size_t>(record_type_t::none));
static_assert(std::ranges::is_sorted(record_type_str));
static_assert(std::ranges::all_of(
		record_type_str, [](const std::string_view view) { return view.size() == section_id_byte_size; }
));

static_assert(record_type_str[static_cast<std::size_t>(record_type_t::aact)] == "AACT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::achr)] == "ACHR");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::acti)] == "ACTI");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::addn)] == "ADDN");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::alch)] == "ALCH");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::ammo)] == "AMMO");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::anio)] == "ANIO");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::appa)] == "APPA");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::arma)] == "ARMA");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::armo)] == "ARMO");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::arto)] == "ARTO");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::aspc)] == "ASPC");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::astp)] == "ASTP");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::avif)] == "AVIF");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::book)] == "BOOK");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::bptd)] == "BPTD");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::cams)] == "CAMS");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::cell)] == "CELL");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::clas)] == "CLAS");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::clfm)] == "CLFM");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::clmt)] == "CLMT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::cobj)] == "COBJ");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::coll)] == "COLL");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::cont)] == "CONT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::cpth)] == "CPTH");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::csty)] == "CSTY");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::debr)] == "DEBR");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::dial)] == "DIAL");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::dlbr)] == "DLBR");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::dlvw)] == "DLVW");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::dobj)] == "DOBJ");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::door)] == "DOOR");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::dual)] == "DUAL");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::eczn)] == "ECZN");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::efsh)] == "EFSH");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::ench)] == "ENCH");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::equp)] == "EQUP");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::expl)] == "EXPL");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::eyes)] == "EYES");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::fact)] == "FACT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::flor)] == "FLOR");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::flst)] == "FLST");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::fstp)] == "FSTP");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::fsts)] == "FSTS");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::furn)] == "FURN");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::glob)] == "GLOB");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::gmst)] == "GMST");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::gras)] == "GRAS");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::grup)] == "GRUP");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::hazd)] == "HAZD");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::hdpt)] == "HDPT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::idle)] == "IDLE");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::idlm)] == "IDLM");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::imad)] == "IMAD");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::imgs)] == "IMGS");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::info)] == "INFO");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::ingr)] == "INGR");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::ipct)] == "IPCT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::ipds)] == "IPDS");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::keym)] == "KEYM");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::kywd)] == "KYWD");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::land)] == "LAND");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::lcrt)] == "LCRT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::lctn)] == "LCTN");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::lgtm)] == "LGTM");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::ligh)] == "LIGH");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::lscr)] == "LSCR");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::ltex)] == "LTEX");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::lvli)] == "LVLI");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::lvln)] == "LVLN");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::lvsp)] == "LVSP");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::mato)] == "MATO");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::matt)] == "MATT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::mesg)] == "MESG");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::mgef)] == "MGEF");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::misc)] == "MISC");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::movt)] == "MOVT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::mstt)] == "MSTT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::musc)] == "MUSC");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::must)] == "MUST");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::navi)] == "NAVI");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::navm)] == "NAVM");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::note)] == "NOTE");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::npc)] == "NPC_");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::otft)] == "OTFT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::pack)] == "PACK");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::perk)] == "PERK");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::pgre)] == "PGRE");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::phzd)] == "PHZD");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::proj)] == "PROJ");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::qust)] == "QUST");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::race)] == "RACE");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::refr)] == "REFR");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::regn)] == "REGN");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::rela)] == "RELA");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::revb)] == "REVB");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::rfct)] == "RFCT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::scen)] == "SCEN");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::scrl)] == "SCRL");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::shou)] == "SHOU");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::slgm)] == "SLGM");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::smbn)] == "SMBN");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::smen)] == "SMEN");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::smqn)] == "SMQN");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::snct)] == "SNCT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::sndr)] == "SNDR");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::sopm)] == "SOPM");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::soun)] == "SOUN");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::spel)] == "SPEL");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::spgd)] == "SPGD");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::stat)] == "STAT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::tact)] == "TACT");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::tes4)] == "TES4");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::tree)] == "TREE");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::txst)] == "TXST");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::vtyp)] == "VTYP");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::watr)] == "WATR");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::weap)] == "WEAP");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::woop)] == "WOOP");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::wrld)] == "WRLD");
static_assert(record_type_str[static_cast<std::size_t>(record_type_t::wthr)] == "WTHR");

// field_type_str maps each valid field_type enum value with its TES char representation.
static_assert(field_type_str.size() == static_cast<std::size_t>(field_type_t::none));
static_assert(std::ranges::is_sorted(field_type_str));
static_assert(
		std::ranges::all_of(field_type_str, [](const std::string_view view) { return view.size() == section_id_byte_size; })
);

static_assert(field_type_str[static_cast<std::size_t>(field_type_t::cnam)] == "CNAM");
static_assert(field_type_str[static_cast<std::size_t>(field_type_t::desc)] == "DESC");
static_assert(field_type_str[static_cast<std::size_t>(field_type_t::edid)] == "EDID");
static_assert(field_type_str[static_cast<std::size_t>(field_type_t::full)] == "FULL");

}
