#include "rank_support.h"


class select_support
{
    private:
        rank_support *r;    // Rank support on which this select support functions.

        uint64_t select(uint64_t rank, bool bit);

    public:
        select_support() {}
        select_support(rank_support *R) { build(R); }

        void build(rank_support *R) { r = R; }
        uint64_t select1(uint64_t rank);
        uint64_t select0(uint64_t rank);
        uint64_t overhead();
};



uint64_t select_support::select(uint64_t rank, bool bit)
{
    if(!rank || rank > r -> bitvector_len())
        return std::numeric_limits<uint64_t>::max();

    uint64_t low = 0, high = r -> bitvector_len() - 1, mid, soln;

    soln = std::numeric_limits<uint64_t>::max();
    while(low <= high)
    {
        mid = (low + high) / 2;
        uint64_t rankMid = (bit ? r -> rank1(mid) : r -> rank0(mid));

        if(rankMid < rank)
            low = mid + 1;
        else if(rankMid > rank)
            high = mid - 1;
        else
        {
            soln = mid;

            if(mid > 0)
                high = mid - 1;
            else
                break;
        }
    }

    return soln;
}



uint64_t select_support::select1(uint64_t rank)
{
    return select(rank, 1);
}



uint64_t select_support::select0(uint64_t rank)
{
    return select(rank, 0);
}



uint64_t select_support::overhead()
{
    return 0;
}