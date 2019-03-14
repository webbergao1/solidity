contract Test {
	struct S { uint8 a; mapping(uint => uint) b; uint8 c; }
	S s;
	function f() public {
		S memory x;
		x.b[1];
	}
}
// ----
// TypeError: (104-114): Type struct Test.S memory is only valid in storage. Mappings cannot live outside storage.
// TypeError: (118-121): Member "b" is not available in struct Test.S memory outside of storage.
