# Copyright 2024 Braden Ganetsky
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

import gdb.printing

class BoostUnorderedHelpers:
    def maybe_unwrap_atomic(n):
        if f"{n.type.strip_typedefs()}".startswith("std::atomic<"):
            underlying_type = n.type.template_argument(0)
            return n.cast(underlying_type)
        else:
            return n
            
    def maybe_unwrap_foa_element(e):
        if f"{e.type.strip_typedefs()}".startswith("boost::unordered::detail::foa::element_type<"):
            return e["p"]
        else:
            return e

    def countr_zero(n):
        for i in range(32):
            if (n & (1 << i)) != 0:
                return i
        return 32

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

class BoostUnorderedFoaPrinter:
    def __init__(self, val):
        self.val = val
        self.name = f"{self.val.type.strip_typedefs()}".split("<")[0]
        self.name = self.name.replace("boost::unordered::", "boost::")
        self.is_map = self.name.endswith("map")

    def to_string(self):
        size = BoostUnorderedHelpers.maybe_unwrap_atomic(self.val["table_"]["size_ctrl"]["size"])
        return f"{self.name} with {size} elements"

    def display_hint(self):
        return "map"

    def to_address(self, arg):
        return arg # TODO: Add fancy pointer support

    def next(self, arg, n):
        return arg + n # TODO: Add fancy pointer support

    def is_regular_layout(self, group):
        typename = group["m"].type.strip_typedefs()
        array_size = typename.sizeof // typename.target().sizeof
        if array_size == 16:
            return True
        elif array_size == 2:
            return False

    def match_occupied(self, group):
        m = group["m"]
        at = lambda b: BoostUnorderedHelpers.maybe_unwrap_atomic(m[b]["n"])

        if self.is_regular_layout(group):
            bits = [1 << b for b in range(16) if at(b) == 0]
            return 0x7FFF & ~sum(bits)
        else:
            xx = at(0) | at(1)
            yy = xx | (xx >> 32)
            return 0x7FFF & (yy | (yy >> 16))

    def is_sentinel(self, group, pos):
        m = group["m"]
        at = lambda b: BoostUnorderedHelpers.maybe_unwrap_atomic(m[b]["n"])

        N = group["N"]
        sentinel_ = group["sentinel_"]
        if self.is_regular_layout(group):
            return pos == N-1 and at(N-1) == sentinel_
        else:
            return pos == N-1 and (at(0) & 0x4000400040004000) == 0x4000 and (at(1) & 0x4000400040004000) == 0

    def children(self):
        def generator():
            table = self.val["table_"]
            groups = self.to_address(table["arrays"]["groups_"])
            elements = self.to_address(table["arrays"]["elements_"])

            pc_ = groups.cast(gdb.lookup_type("unsigned char").pointer())
            p_ = elements
            first_time = True
            mask = 0
            n0 = 0
            n = 0

            count = 0
            while p_ != 0:
                # This if block mirrors the condition in the begin() call
                if (not first_time) or (self.match_occupied(groups.dereference()) & 1):
                    pointer = BoostUnorderedHelpers.maybe_unwrap_foa_element(p_)
                    value = self.to_address(pointer).dereference()
                    if self.is_map:
                        first = value["first"]
                        second = value["second"]
                        yield "", first
                        yield "", second
                    else:
                        yield "", count
                        yield "", value
                    count += 1
                first_time = False

                n0 = pc_.cast(gdb.lookup_type("uintptr_t")) % groups.dereference().type.sizeof
                pc_ = self.next(pc_, -n0)

                mask = (self.match_occupied(pc_.cast(groups.type).dereference()) >> (n0+1)) << (n0+1)
                while mask == 0:
                    pc_ = self.next(pc_, groups.dereference().type.sizeof)
                    p_ = self.next(p_, groups.dereference()["N"])
                    mask = self.match_occupied(pc_.cast(groups.type).dereference())
                
                n = BoostUnorderedHelpers.countr_zero(mask)
                if self.is_sentinel(pc_.cast(groups.type).dereference(), n):
                    p_ = 0
                else:
                    pc_ = self.next(pc_, n)
                    p_ = self.next(p_, n - n0)

        return generator()

class BoostUnorderedFoaIteratorPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        if self.valid():
            pointer = BoostUnorderedHelpers.maybe_unwrap_foa_element(self.val["p_"])
            value = self.to_address(pointer).dereference()
            return f"iterator = {{ {value} }}"
        else:
            return "iterator = { end iterator }"

    def to_address(self, arg):
        return arg # TODO: Add fancy pointer support

    def valid(self):
        return (self.to_address(self.val["p_"]) != 0) and (self.to_address(self.val["pc_"]) != 0)

def boost_unordered_build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("boost_unordered")
    add_template_printer = lambda name, printer: pp.add_printer(name, f"^{name}<.*>$", printer)

    add_template_printer("boost::unordered::unordered_map", BoostUnorderedFcaPrinter)
    add_template_printer("boost::unordered::unordered_multimap", BoostUnorderedFcaPrinter)
    add_template_printer("boost::unordered::unordered_set", BoostUnorderedFcaPrinter)
    add_template_printer("boost::unordered::unordered_multiset", BoostUnorderedFcaPrinter)

    add_template_printer("boost::unordered::detail::iterator_detail::iterator", BoostUnorderedFcaIteratorPrinter)
    add_template_printer("boost::unordered::detail::iterator_detail::c_iterator", BoostUnorderedFcaIteratorPrinter)

    add_template_printer("boost::unordered::unordered_flat_map", BoostUnorderedFoaPrinter)
    add_template_printer("boost::unordered::unordered_flat_set", BoostUnorderedFoaPrinter)
    add_template_printer("boost::unordered::unordered_node_map", BoostUnorderedFoaPrinter)
    add_template_printer("boost::unordered::unordered_node_set", BoostUnorderedFoaPrinter)
    add_template_printer("boost::unordered::concurrent_flat_map", BoostUnorderedFoaPrinter)
    add_template_printer("boost::unordered::concurrent_flat_set", BoostUnorderedFoaPrinter)
    
    add_template_printer("boost::unordered::detail::foa::table_iterator", BoostUnorderedFoaIteratorPrinter)

    return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), boost_unordered_build_pretty_printer())
