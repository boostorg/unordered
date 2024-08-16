# Copyright 2024 Braden Ganetsky
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

import gdb.printing

class BoostUnorderedFcaPrinter:
    def __init__(self, val):
        self.val = val
        self.name = f"{self.val.type.strip_typedefs()}".split("<")[0]
        self.name = self.name.replace("boost::unordered::", "boost::")
        self.is_map = self.name.endswith("map")

    def to_string(self):
        size = self.val["table_"]["size_"]
        return f"{self.name} with {size} elements"

    def display_hint(self):
        return "map"

    def to_address(self, arg):
        return arg # TODO: Add fancy pointer support

    def next(self, arg, n):
        return arg + n # TODO: Add fancy pointer support

    def children(self):
        def generator():
            grouped_buckets = self.val["table_"]["buckets_"]

            size = grouped_buckets["size_"]
            buckets = grouped_buckets["buckets"]
            bucket_index = 0
            current_bucket = self.to_address(buckets)
            node = self.to_address(current_bucket.dereference()["next"])

            count = 0
            while bucket_index != size:
                current_bucket = self.next(self.to_address(buckets), bucket_index)
                node = self.to_address(current_bucket.dereference()["next"])
                while node != 0:
                    value = node.dereference()["buf"]["t_"]
                    if self.is_map:
                        first = value["first"]
                        second = value["second"]
                        yield "", first
                        yield "", second
                    else:
                        yield "", count
                        yield "", value
                    count += 1
                    node = self.to_address(node.dereference()["next"])
                bucket_index += 1
        
        return generator()

class BoostUnorderedFcaIteratorPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        if self.valid():
            value = self.to_address(self.val["p"]).dereference()["buf"]["t_"]
            return f"iterator = {{ {value} }}"
        else:
            return "iterator = { end iterator }"

    def to_address(self, arg):
        return arg # TODO: Add fancy pointer support

    def valid(self):
        return (self.to_address(self.val["p"]) != 0) and (self.to_address(self.val["itb"]["p"]) != 0)

def boost_unordered_build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("boost_unordered")
    add_template_printer = lambda name, printer: pp.add_printer(name, f"^{name}<.*>$", printer)

    add_template_printer("boost::unordered::unordered_map", BoostUnorderedFcaPrinter)
    add_template_printer("boost::unordered::unordered_multimap", BoostUnorderedFcaPrinter)
    add_template_printer("boost::unordered::unordered_set", BoostUnorderedFcaPrinter)
    add_template_printer("boost::unordered::unordered_multiset", BoostUnorderedFcaPrinter)

    add_template_printer("boost::unordered::detail::iterator_detail::iterator", BoostUnorderedFcaIteratorPrinter)
    add_template_printer("boost::unordered::detail::iterator_detail::c_iterator", BoostUnorderedFcaIteratorPrinter)

    return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), boost_unordered_build_pretty_printer())
