#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace josk::tes
{

// For an in-depth description of the format, check the UESP documentation at:
// https://en.uesp.net/wiki/Skyrim_Mod:Mod_File_Format

/** In josk, parsed record types are represented as a single byte. */
enum class record_type_t : std::uint8_t
{
	// clang-format off
	aact, achr, acti, addn, alch, ammo, anio, appa, arma, armo, arto, aspc, astp, avif,
	book, bptd, cams, cell, clas, clfm, clmt, cobj, coll, cont, cpth, csty, debr, dial,
	dlbr, dlvw, dobj, door, dual, eczn, efsh, ench, equp, expl, eyes, fact, flor, flst,
	fstp, fsts, furn, glob, gmst, gras, grup, hazd, hdpt, idle, idlm, imad, imgs, info,
	ingr, ipct, ipds, keym, kywd, land, lcrt, lctn, lgtm, ligh, lscr, ltex, lvli, lvln,
	lvsp, mato, matt, mesg, mgef, misc, movt, mstt, musc, must, navi, navm, note, npc,
	otft, pack, perk, pgre, phzd, proj, qust, race, refr, regn, rela, revb, rfct, scen,
	scrl, shou, slgm, smbn, smen, smqn, snct, sndr, sopm, soun, spel, spgd, stat, tact,
	tes4, tree, txst, vtyp, watr, weap, woop, wrld, wthr,
	// clang-format on
	// Used by josk to represent end of file, errors, and other conditions.
	none,
};

/** String representations of record types, as they appear in TES files. Indexed by their record_type_t. */
constexpr std::array<std::string_view, 121Z> record_type_str{
		"AACT", "ACHR", "ACTI", "ADDN", "ALCH", "AMMO", "ANIO", "APPA", "ARMA", "ARMO", "ARTO", "ASPC", "ASTP", "AVIF",
		"BOOK", "BPTD", "CAMS", "CELL", "CLAS", "CLFM", "CLMT", "COBJ", "COLL", "CONT", "CPTH", "CSTY", "DEBR", "DIAL",
		"DLBR", "DLVW", "DOBJ", "DOOR", "DUAL", "ECZN", "EFSH", "ENCH", "EQUP", "EXPL", "EYES", "FACT", "FLOR", "FLST",
		"FSTP", "FSTS", "FURN", "GLOB", "GMST", "GRAS", "GRUP", "HAZD", "HDPT", "IDLE", "IDLM", "IMAD", "IMGS", "INFO",
		"INGR", "IPCT", "IPDS", "KEYM", "KYWD", "LAND", "LCRT", "LCTN", "LGTM", "LIGH", "LSCR", "LTEX", "LVLI", "LVLN",
		"LVSP", "MATO", "MATT", "MESG", "MGEF", "MISC", "MOVT", "MSTT", "MUSC", "MUST", "NAVI", "NAVM", "NOTE", "NPC_",
		"OTFT", "PACK", "PERK", "PGRE", "PHZD", "PROJ", "QUST", "RACE", "REFR", "REGN", "RELA", "REVB", "RFCT", "SCEN",
		"SCRL", "SHOU", "SLGM", "SMBN", "SMEN", "SMQN", "SNCT", "SNDR", "SOPM", "SOUN", "SPEL", "SPGD", "STAT", "TACT",
		"TES4", "TREE", "TXST", "VTYP", "WATR", "WEAP", "WOOP", "WRLD", "WTHR",
};

/**
 * Converts a char[4] into the record type representation used by josk.
 * @param record_type_string Must have a size of 4Z.
 * @return none if any error occurred, record_type otherwise.
 */
[[nodiscard]] record_type_t to_record_type(std::string_view record_type_string) noexcept;

/**
 * Returns the string representation of a record type.
 * @param record_type Type to check.
 * @return String representation if record_type is valid, NONE otherwise.
 */
[[nodiscard]] constexpr std::string_view to_record_string(record_type_t record_type) noexcept
{
	const auto record_type_index = static_cast<std::size_t>(record_type);
	if (record_type_index >= record_type_str.size())
	{
		return "NONE";
	}

	return record_type_str[record_type_index];
}

/** Expresses record, group or field id sizes in a TES file. */
constexpr std::size_t section_id_byte_size = 4Z;

/** Form (or record) identifiers are unique identifiers for individual records. */
using formid_t = std::uint32_t;

/** Parsed field types are represented as a single byte. */
enum class field_type_t : std::uint8_t
{
	anam,
	avsk,
	cnam,
	ctda,
	data,
	desc,
	edid,
	fnam,
	full,
	hnam,
	icon,
	inam,
	pnam,
	snam,
	vmad,
	vnam,
	xnam,
	ynam,
	none
};

/** String representations of field types, as they appear in TES files. Indexed by their field_type_t. */
constexpr std::array<std::string_view, 18Z> field_type_str{
		"ANAM", "AVSK", "CNAM", "CTDA", "DATA", "DESC", "EDID", "FNAM", "FULL",
		"HNAM", "ICON", "INAM", "PNAM", "SNAM", "VMAD", "VNAM", "XNAM", "YNAM",
};

/**
 * Returns the string representation of a field type.
 * @param field_type Type to check.
 * @return String representation if field_type is valid, NONE otherwise.
 */
[[nodiscard]] constexpr std::string_view to_field_string(field_type_t field_type) noexcept
{
	const auto field_type_index = static_cast<std::size_t>(field_type);
	if (field_type_index >= field_type_str.size())
	{
		return "NONE";
	}

	return field_type_str[field_type_index];
}

enum class skill_category_t : std::uint8_t
{
	other = 0U,
	combat = 1U,
	magic = 2U,
	stealth = 3U,
};

struct perk_record final
{
	formid_t record_id;
	std::string name;
	std::string description;
	std::uint8_t skill_req;
	std::vector<formid_t> prereq_perk_ids;
	formid_t next_perk_id;
};

struct avif_perk final
{
	formid_t record_id;
	float x_pos;
	float y_pos;
};

struct avif_record final
{
	formid_t record_id;
	std::string name;
	std::string description;
	skill_category_t category;
	std::vector<avif_perk> perks;
};

}
