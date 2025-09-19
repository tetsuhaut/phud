#include "entities/Card.hpp" // Card, std::string_view
#include "language/EnumMapper.hpp"

static constexpr auto CARD_MAPPER = makeEnumMapper<Card, 53>({{
  {Card::none, "none"}, {Card::twoSpade, "2s"}, {Card::threeSpade, "3s"},
  {Card::fourSpade, "4s"}, {Card::fiveSpade, "5s"}, {Card::sixSpade, "6s"},
  {Card::sevenSpade, "7s"}, {Card::eightSpade, "8s"}, {Card::nineSpade, "9s"},
  {Card::tenSpade, "Ts"}, {Card::jackSpade, "Js"}, {Card::queenSpade, "Qs"},
  {Card::kingSpade, "Ks"}, {Card::aceSpade, "As"}, {Card::twoHeart, "2h"},
  {Card::threeHeart, "3h"}, {Card::fourHeart, "4h"}, {Card::fiveHeart, "5h"},
  {Card::sixHeart, "6h"}, {Card::sevenHeart, "7h"}, {Card::eightHeart, "8h"},
  {Card::nineHeart, "9h"}, {Card::tenHeart, "Th"}, {Card::jackHeart, "Jh"},
  {Card::queenHeart, "Qh"}, {Card::kingHeart, "Kh"}, {Card::aceHeart, "Ah"},
  {Card::twoDiamond, "2d"}, {Card::threeDiamond, "3d"}, {Card::fourDiamond, "4d"},
  {Card::fiveDiamond, "5d"}, {Card::sixDiamond, "6d"}, {Card::sevenDiamond, "7d"},
  {Card::eightDiamond, "8d"}, {Card::nineDiamond, "9d"}, {Card::tenDiamond, "Td"},
  {Card::jackDiamond, "Jd"}, {Card::queenDiamond, "Qd"}, {Card::kingDiamond, "Kd"},
  {Card::aceDiamond, "Ad"}, {Card::twoClub, "2c"}, {Card::threeClub, "3c"},
  {Card::fourClub, "4c"}, {Card::fiveClub, "5c"}, {Card::sixClub, "6c"},
  {Card::sevenClub, "7c"}, {Card::eightClub, "8c"}, {Card::nineClub, "9c"},
  {Card::tenClub, "Tc"}, {Card::jackClub, "Jc"}, {Card::queenClub, "Qc"},
  {Card::kingClub, "Kc"}, {Card::aceClub, "Ac"}
}});

std::string_view toString(Card c) {
  return CARD_MAPPER.toString(c);
}

Card toCard(std::string_view strCard) {
  return CARD_MAPPER.fromString(strCard);
}
