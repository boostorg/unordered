#include <boost/concept_check.hpp>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

int main()
{
    using namespace boost;

    typedef boost::unordered_set<int> UnorderedSet;
    typedef boost::unordered_multiset<int> UnorderedMultiSet;
    typedef boost::unordered_map<int, int> UnorderedMap;
    typedef boost::unordered_multimap<int, int> UnorderedMultiMap;

    function_requires< UnorderedAssociativeContainerConcept<UnorderedSet> >();
    function_requires< SimpleAssociativeContainerConcept<UnorderedSet> >();
    function_requires< UniqueAssociativeContainerConcept<UnorderedSet> >();

    function_requires< UnorderedAssociativeContainerConcept<UnorderedMultiSet> >();
    function_requires< SimpleAssociativeContainerConcept<UnorderedMultiSet> >();
    function_requires< MultipleAssociativeContainerConcept<UnorderedMultiSet> >();

    function_requires< UnorderedAssociativeContainerConcept<UnorderedMap> >();
    function_requires< UniqueAssociativeContainerConcept<UnorderedMap> >();
    function_requires< PairAssociativeContainerConcept<UnorderedMap> >();

    function_requires< UnorderedAssociativeContainerConcept<UnorderedMultiMap> >();
    function_requires< MultipleAssociativeContainerConcept<UnorderedMultiMap> >();
    function_requires< PairAssociativeContainerConcept<UnorderedMultiMap> >();

    return 0;
}
