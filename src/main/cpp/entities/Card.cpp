#include "entities/Card.hpp" // Card, std::string_view
#include "language/EnumMapper.hpp"

static constexpr auto CARD_MAPPER = makeEnumMapper<Card>(
  std::pair{Card::none, "none"}, std::pair{Card::twoSpade, "2s"}, std::pair{Card::threeSpade, "3s"},
  std::pair{Card::fourSpade, "4s"}, std::pair{Card::fiveSpade, "5s"}, std::pair{Card::sixSpade, "6s"},
  std::pair{Card::sevenSpade, "7s"}, std::pair{Card::eightSpade, "8s"}, std::pair{Card::nineSpade, "9s"},
  std::pair{Card::tenSpade, "Ts"}, std::pair{Card::jackSpade, "Js"}, std::pair{Card::queenSpade, "Qs"},
  std::pair{Card::kingSpade, "Ks"}, std::pair{Card::aceSpade, "As"}, std::pair{Card::twoHeart, "2h"},
  std::pair{Card::threeHeart, "3h"}, std::pair{Card::fourHeart, "4h"}, std::pair{Card::fiveHeart, "5h"},
  std::pair{Card::sixHeart, "6h"}, std::pair{Card::sevenHeart, "7h"}, std::pair{Card::eightHeart, "8h"},
  std::pair{Card::nineHeart, "9h"}, std::pair{Card::tenHeart, "Th"}, std::pair{Card::jackHeart, "Jh"},
  std::pair{Card::queenHeart, "Qh"}, std::pair{Card::kingHeart, "Kh"}, std::pair{Card::aceHeart, "Ah"},
  std::pair{Card::twoDiamond, "2d"}, std::pair{Card::threeDiamond, "3d"}, std::pair{Card::fourDiamond, "4d"},
  std::pair{Card::fiveDiamond, "5d"}, std::pair{Card::sixDiamond, "6d"}, std::pair{Card::sevenDiamond, "7d"},
  std::pair{Card::eightDiamond, "8d"}, std::pair{Card::nineDiamond, "9d"}, std::pair{Card::tenDiamond, "Td"},
  std::pair{Card::jackDiamond, "Jd"}, std::pair{Card::queenDiamond, "Qd"}, std::pair{Card::kingDiamond, "Kd"},
  std::pair{Card::aceDiamond, "Ad"}, std::pair{Card::twoClub, "2c"}, std::pair{Card::threeClub, "3c"},
  std::pair{Card::fourClub, "4c"}, std::pair{Card::fiveClub, "5c"}, std::pair{Card::sixClub, "6c"},
  std::pair{Card::sevenClub, "7c"}, std::pair{Card::eightClub, "8c"}, std::pair{Card::nineClub, "9c"},
  std::pair{Card::tenClub, "Tc"}, std::pair{Card::jackClub, "Jc"}, std::pair{Card::queenClub, "Qc"},
  std::pair{Card::kingClub, "Kc"}, std::pair{Card::aceClub, "Ac"}
);

std::string_view toString(Card c) {
  return CARD_MAPPER.toString(c);
}

Card toCard(std::string_view strCard) {
  return CARD_MAPPER.fromString(strCard);
}
