#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace josk::tes
{

// Only the parts of the format visible outside the parser are exposed in this interface.
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

/**
 * Converts a char[4] into the record type representation used by josk.
 * @param record_type_string Must have a size of 4Z.
 * @return none if any error occurred, record_type otherwise.
 */
record_type_t to_record_type(std::string_view record_type_string) noexcept;

/**
 * Shows the string representation of a record type. Intended for error reporting.
 * @param record_type Type to check.
 * @return String representation if record_type is valid, NONE otherwise.
 */
std::string_view to_record_string(record_type_t record_type) noexcept;

/** Expresses record and group data sizes in a TES file. */
using record_size_t = std::uint32_t;
constexpr record_size_t record_type_size = 4U;

/** Form (or record) identifiers are unique ids for individual records. */
using formid_t = std::uint32_t;

struct avif_record final
{
	formid_t formid;
	std::string name;
	std::string description;
};

}
