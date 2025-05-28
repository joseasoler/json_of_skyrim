#pragma once

#include <josk/tes_helper.hpp>

#include <cstdint>

namespace josk::tes
{

// For an in-depth description of the format, check the UESP documentation at:
// https://en.uesp.net/wiki/Skyrim_Mod:Mod_File_Format

/** Expresses data sizes in a TES file. */
using tes_size_t = std::uint32_t;

/** Form (or record) identifiers are unique ids for individual records. */
using formid_t = std::uint32_t;

/** Each record type has a 4 char unique identifier. josk simplifies processing by keeping them as integers. */
enum class record_type_t : std::uint32_t
{
	// Used by josk to represent end of file, errors, and other conditions.
	none = 0U,

	tes4 = to_record_type("TES4"),
	grup = to_record_type("GRUP"),

	// Most of these will never be used by josk, but they are useful for parsing debugging.
	aact = to_record_type("AACT"),
	achr = to_record_type("ACHR"),
	acti = to_record_type("ACTI"),
	addn = to_record_type("ADDN"),
	alch = to_record_type("ALCH"),
	ammo = to_record_type("AMMO"),
	anio = to_record_type("ANIO"),
	appa = to_record_type("APPA"),
	arma = to_record_type("ARMA"),
	armo = to_record_type("ARMO"),
	arto = to_record_type("ARTO"),
	aspc = to_record_type("ASPC"),
	astp = to_record_type("ASTP"),
	avif = to_record_type("AVIF"),
	book = to_record_type("BOOK"),
	bptd = to_record_type("BPTD"),
	cams = to_record_type("CAMS"),
	cell = to_record_type("CELL"),
	clas = to_record_type("CLAS"),
	clfm = to_record_type("CLFM"),
	clmt = to_record_type("CLMT"),
	cobj = to_record_type("COBJ"),
	coll = to_record_type("COLL"),
	cont = to_record_type("CONT"),
	cpth = to_record_type("CPTH"),
	csty = to_record_type("CSTY"),
	debr = to_record_type("DEBR"),
	dial = to_record_type("DIAL"),
	dlbr = to_record_type("DLBR"),
	dlvw = to_record_type("DLVW"),
	dobj = to_record_type("DOBJ"),
	door = to_record_type("DOOR"),
	dual = to_record_type("DUAL"),
	eczn = to_record_type("ECZN"),
	efsh = to_record_type("EFSH"),
	ench = to_record_type("ENCH"),
	equp = to_record_type("EQUP"),
	expl = to_record_type("EXPL"),
	eyes = to_record_type("EYES"),
	fact = to_record_type("FACT"),
	flor = to_record_type("FLOR"),
	flst = to_record_type("FLST"),
	fstp = to_record_type("FSTP"),
	fsts = to_record_type("FSTS"),
	furn = to_record_type("FURN"),
	glob = to_record_type("GLOB"),
	gmst = to_record_type("GMST"),
	gras = to_record_type("GRAS"),
	hazd = to_record_type("HAZD"),
	hdpt = to_record_type("HDPT"),
	idle = to_record_type("IDLE"),
	idlm = to_record_type("IDLM"),
	imad = to_record_type("IMAD"),
	imgs = to_record_type("IMGS"),
	info = to_record_type("INFO"),
	ingr = to_record_type("INGR"),
	ipct = to_record_type("IPCT"),
	ipds = to_record_type("IPDS"),
	keym = to_record_type("KEYM"),
	kywd = to_record_type("KYWD"),
	land = to_record_type("LAND"),
	lcrt = to_record_type("LCRT"),
	lctn = to_record_type("LCTN"),
	lgtm = to_record_type("LGTM"),
	ligh = to_record_type("LIGH"),
	lscr = to_record_type("LSCR"),
	ltex = to_record_type("LTEX"),
	lvli = to_record_type("LVLI"),
	lvln = to_record_type("LVLN"),
	lvsp = to_record_type("LVSP"),
	mato = to_record_type("MATO"),
	matt = to_record_type("MATT"),
	mesg = to_record_type("MESG"),
	mgef = to_record_type("MGEF"),
	misc = to_record_type("MISC"),
	movt = to_record_type("MOVT"),
	mstt = to_record_type("MSTT"),
	musc = to_record_type("MUSC"),
	must = to_record_type("MUST"),
	navi = to_record_type("NAVI"),
	navm = to_record_type("NAVM"),
	note = to_record_type("NOTE"),
	npc = to_record_type("NPC_"),
	otft = to_record_type("OTFT"),
	pack = to_record_type("PACK"),
	perk = to_record_type("PERK"),
	pgre = to_record_type("PGRE"),
	phzd = to_record_type("PHZD"),
	proj = to_record_type("PROJ"),
	qust = to_record_type("QUST"),
	race = to_record_type("RACE"),
	refr = to_record_type("REFR"),
	regn = to_record_type("REGN"),
	rela = to_record_type("RELA"),
	revb = to_record_type("REVB"),
	rfct = to_record_type("RFCT"),
	scen = to_record_type("SCEN"),
	scrl = to_record_type("SCRL"),
	shou = to_record_type("SHOU"),
	slgm = to_record_type("SLGM"),
	smbn = to_record_type("SMBN"),
	smen = to_record_type("SMEN"),
	smqn = to_record_type("SMQN"),
	snct = to_record_type("SNCT"),
	sndr = to_record_type("SNDR"),
	sopm = to_record_type("SOPM"),
	soun = to_record_type("SOUN"),
	spel = to_record_type("SPEL"),
	spgd = to_record_type("SPGD"),
	stat = to_record_type("STAT"),
	tact = to_record_type("TACT"),
	tree = to_record_type("TREE"),
	txst = to_record_type("TXST"),
	vtyp = to_record_type("VTYP"),
	watr = to_record_type("WATR"),
	weap = to_record_type("WEAP"),
	woop = to_record_type("WOOP"),
	wrld = to_record_type("WRLD"),
	wthr = to_record_type("WTHR")
};

template <typename type>
consteval tes_size_t tes_size_of()
{
	return static_cast<tes_size_t>(sizeof(type));
}

constexpr auto record_type_size = tes_size_of<record_type_t>();

constexpr tes_size_t record_header_size = record_type_size + tes_size_of<tes_size_t>() + tes_size_of<std::uint32_t>() +
																					tes_size_of<formid_t>() + (tes_size_of<std::uint16_t>() * 4U);

constexpr tes_size_t group_header_size = record_type_size + tes_size_of<tes_size_t>() +
																				 (tes_size_of<std::uint8_t>() * 4U) + tes_size_of<std::uint32_t>() +
																				 (tes_size_of<std::uint16_t>() * 2U) + tes_size_of<std::uint32_t>();

}
