// bug #1801
contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    constructor(ActionChoices param) public {}
    function ret() public returns (ActionChoices) {
        ActionChoices action = ActionChoices.GoLeft;
        return action;
    }
}
// ----
//     :test
// [
//   {
//     "constant": false,
//     "inputs": [],
//     "name": "ret",
//     "outputs":
//     [
//       {
//         "internalType": "test.ActionChoices",
//         "name": "",
//         "type": "uint8"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "inputs":
//     [
//       {
//         "internalType": "test.ActionChoices",
//         "name": "param",
//         "type": "uint8"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "constructor"
//   }
// ]
