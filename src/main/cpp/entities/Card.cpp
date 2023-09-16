#include "containers/algorithms.hpp"
#include "entities/Card.hpp" // Card, StringView
#include <frozen/string.h>
#include <frozen/unordered_map.h>

namespace pa = phud::algorithms;

// Note : must use frozen::string when it is a map key.
// frozen::string can be created from StringView.

static constexpr auto STRING_TO_ENUM {
  frozen::make_unordered_map<frozen::string, Card>({
    { "none", Card::none }, { "2s", Card::twoSpade }, { "3s", Card::threeSpade },
    { "4s", Card::fourSpade }, { "5s", Card::fiveSpade },
    { "6s", Card::sixSpade }, { "7s", Card::sevenSpade },
    { "8s", Card::eightSpade }, { "9s", Card::nineSpade },
    { "Ts", Card::tenSpade }, { "Js", Card::jackSpade },
    { "Qs", Card::queenSpade }, { "Ks", Card::kingSpade },
    { "As", Card::aceSpade }, { "2h", Card::twoHeart },
    { "3h", Card::threeHeart }, { "4h", Card::fourHeart },
    { "5h", Card::fiveHeart }, { "6h", Card::sixHeart },
    { "7h", Card::sevenHeart }, { "8h", Card::eightHeart },
    { "9h", Card::nineHeart }, { "Th", Card::tenHeart },
    { "Jh", Card::jackHeart }, { "Qh", Card::queenHeart },
    { "Kh", Card::kingHeart }, { "Ah", Card::aceHeart },
    { "2d", Card::twoDiamond }, { "3d", Card::threeDiamond },
    { "4d", Card::fourDiamond }, { "5d", Card::fiveDiamond },
    { "6d", Card::sixDiamond }, { "7d", Card::sevenDiamond },
    { "8d", Card::eightDiamond }, { "9d", Card::nineDiamond },
    { "Td", Card::tenDiamond }, { "Jd", Card::jackDiamond },
    { "Qd", Card::queenDiamond }, { "Kd", Card::kingDiamond },
    { "Ad", Card::aceDiamond }, { "2c", Card::twoClub },
    { "3c", Card::threeClub }, { "4c", Card::fourClub },
    { "5c", Card::fiveClub }, { "6c", Card::sixClub },
    { "7c", Card::sevenClub }, { "8c", Card::eightClub },
    { "9c", Card::nineClub }, { "Tc", Card::tenClub },
    { "Jc", Card::jackClub }, { "Qc", Card::queenClub },
    { "Kc", Card::kingClub }, { "Ac", Card::aceClub }
  })
};

static constexpr auto ENUM_TO_STRING {
  frozen::make_unordered_map<Card, StringView>({
    { Card::none, "none" }, { Card::twoSpade, "2s" }, { Card::threeSpade, "3s" },
    { Card::fourSpade, "4s" }, { Card::fiveSpade, "5s" },
    { Card::sixSpade, "6s" }, { Card::sevenSpade, "7s" },
    { Card::eightSpade, "8s" }, { Card::nineSpade, "9s" },
    { Card::tenSpade, "Ts" }, { Card::jackSpade, "Js" },
    { Card::queenSpade, "Qs" }, { Card::kingSpade, "Ks" },
    { Card::aceSpade, "As" }, { Card::twoHeart, "2h" },
    { Card::threeHeart, "3h" }, { Card::fourHeart, "4h" },
    { Card::fiveHeart, "5h" }, { Card::sixHeart, "6h" },
    { Card::sevenHeart, "7h" }, { Card::eightHeart, "8h" },
    { Card::nineHeart, "9h" }, { Card::tenHeart, "Th" },
    { Card::jackHeart, "Jh" }, { Card::queenHeart, "Qh" },
    { Card::kingHeart, "Kh" }, { Card::aceHeart, "Ah" },
    { Card::twoDiamond, "2d" }, { Card::threeDiamond, "3d" },
    { Card::fourDiamond, "4d" }, { Card::fiveDiamond, "5d" },
    { Card::sixDiamond, "6d" }, { Card::sevenDiamond, "7d" },
    { Card::eightDiamond, "8d" }, { Card::nineDiamond, "9d" },
    { Card::tenDiamond, "Td" }, { Card::jackDiamond, "Jd" },
    { Card::queenDiamond, "Qd" }, { Card::kingDiamond, "Kd" },
    { Card::aceDiamond, "Ad" }, { Card::twoClub, "2c" },
    { Card::threeClub, "3c" }, { Card::fourClub, "4c" },
    { Card::fiveClub, "5c" }, { Card::sixClub, "6c" },
    { Card::sevenClub, "7c" }, { Card::eightClub, "8c" },
    { Card::nineClub, "9c" }, { Card::tenClub, "Tc" },
    { Card::jackClub, "Jc" }, { Card::queenClub, "Qc" },
    { Card::kingClub, "Kc" }, { Card::aceClub, "Ac" }
  })
};

StringView toString(Card c) {
  return pa::getValueFromKey(ENUM_TO_STRING, c);
}

Card toCard(StringView strCard) {
  return pa::getValueFromKey(STRING_TO_ENUM, strCard);
}
