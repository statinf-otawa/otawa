#include <otawa/ai/PseudoTopoOrder.h>

using namespace elm;
using namespace otawa;

namespace otawa { namespace ai {

void PseudoTopoOrder::_topoLoopHelper(const CFGGraph &graph, Block *start, int currentLoop) {
	for (ai::CFGGraph::Iterator it(graph); !it.ended(); it++) {
		Block *bb = (*it);
		if (LOOP_HEADER(bb)) {
			if (((currentLoop == -1) && (ENCLOSING_LOOP_HEADER(bb) == nullptr)) ||
				((currentLoop != -1) && (ENCLOSING_LOOP_HEADER(bb) != nullptr) && (ENCLOSING_LOOP_HEADER(bb)->index() == currentLoop))) {
				_topoLoopHelper(graph, bb, bb->index());
			}
		}
	}

	if (currentLoop != -1) {
		_loopOrder[currentLoop] = _current;
		_current++;
	}
}

bool PseudoTopoOrder::isBefore(const Block *b1, const Block *b2) const {
	if (_belongsToLoop.hasKey(b1->index()) && !_belongsToLoop.hasKey(b2->index()))
		return true;

	if (!_belongsToLoop.hasKey(b1->index()) && _belongsToLoop.hasKey(b2->index()))
		return false;

	if (_belongsToLoop.hasKey(b1->index())) {
		int loopId = _belongsToLoop[b1->index()];

		if (_loopOrder[loopId] < _loopOrder[loopId]) 
			return true;

		if (_loopOrder[loopId] > _loopOrder[loopId]) 
			return false;
	}

	return (_blockOrder[b1->index()] < _blockOrder[b2->index()]);
}

void PseudoTopoOrder::_topoNodeHelper(const CFGGraph &graph, Block *end) {
	if (_visited->bit(end->index()))
		return;

	for (ai::CFGGraph::Predecessor e(graph, end); e(); e++) {
		if (!BACK_EDGE(*e))
			_topoNodeHelper(graph, e->source());
	}

	if (LOOP_HEADER(end)) {
		_belongsToLoop[end->index()] = end->index();
	} else if (ENCLOSING_LOOP_HEADER(end) != nullptr)
		_belongsToLoop[end->index()] = ENCLOSING_LOOP_HEADER(end)->index();

	_blockOrder[end->index()] = _current;
	_current++;
	_visited->set(end->index());
}

void PseudoTopoOrder::_getPseudoTopo(const CFGGraph &graph) {
	_visited = new BitVector(graph.count());

	_current = 0;
	_topoLoopHelper(graph, graph.entry(), -1);

	_current = 0;
	for (ai::CFGGraph::Iterator it(graph); !it.ended(); it++) {
		bool hasNonBackEdges = false;
		for (ai::CFGGraph::Successor e(graph, (*it)); !e.ended() && !hasNonBackEdges; e++) {
			if (!BACK_EDGE(*e))
				hasNonBackEdges = true;
		}
		if (!hasNonBackEdges)
			_topoNodeHelper(graph, *it);
	}
	delete _visited;
	_visited = nullptr;
}

} }
