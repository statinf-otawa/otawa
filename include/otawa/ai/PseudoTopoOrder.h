#include <elm/data/HashMap.h>
#include <elm/util/BitVector.h>

#include <otawa/ai/CFGGraph.h>

namespace otawa { namespace ai {

	class PseudoTopoOrder {
		private:
			void _getPseudoTopo(const otawa::ai::CFGGraph &graph);
			void _topoLoopHelper(const otawa::ai::CFGGraph &graph, Block *start, int currentLoop);
			void _topoNodeHelper(const otawa::ai::CFGGraph &graph, Block *end);
			elm::HashMap<int, int> _loopOrder;
			elm::HashMap<int, int> _blockOrder;
			elm::HashMap<int, int> _belongsToLoop;
			BitVector *_visited{};
			int _current;

		public:

			/**
			 * @param graph The CFG to order
			 */
			PseudoTopoOrder(const otawa::ai::CFGGraph &graph) {
				_getPseudoTopo(graph);
			}

			/**
			 * Determines the preferred processing order in the abstract interpretation
			 * 
			 * @param b1 A basic block
			 * @param b2 Another basic block
			 * @return true if b1 should be processed before b2
			 */
			bool isBefore(const Block *b1, const Block *b2) const;
	};
} } 