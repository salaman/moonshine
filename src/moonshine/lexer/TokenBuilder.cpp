#include "TokenBuilder.h"

TokenBuilder TokenBuilder::define(int tokenId)
{
    return TokenBuilder(tokenId);
}

TokenBuilder::TokenBuilder(int tokenId)
    : tokenId_(tokenId)
{
}

TokenBuilder TokenBuilder::with(std::function<void(int&)> builderFunc)
{
    int a;

    builderFunc(a);

    return TokenBuilder(0);
}
