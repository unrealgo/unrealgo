
#include <tensorflow/c/c_api_internal.h>
#include <tensorflow/cc/framework/scope.h>
#include <fstream>
#include <tensorflow/core/graph/graph_def_builder.h>
#include <deque>
#include <tensorflow/core/graph/algorithm.h>
#include "DlGraphUtil.h"

using namespace tensorflow;

namespace DlGraphUtil {

void ClearInput(GraphDef& graphDef, const std::string& nodeName) {
  GetNode(graphDef, nodeName)->clear_input();
}

void OptimizeGraph(GraphDef& graphDef, std::vector<std::string>& outNodeNames, const std::string& inputName) {
  DlGraphUtil::PruneNodesUnReachableTo(graphDef, outNodeNames);
  DlGraphUtil::ClearInput(graphDef, inputName);
}

void PruneForReverseReachability(GraphDef& graphDef, std::vector<std::string>& nodeNames) {
  Graph g(OpRegistry::Global());
  DlGraphUtil::GraphDef2Graph(graphDef, &g);
  std::unordered_set<const Node*> nodes;
  for (auto& name : nodeNames) {
    nodes.insert(DlGraphUtil::GetNode(g, name));
    nodes.insert(DlGraphUtil::GetNode(g, name));
  }

  PruneForReverseReachability(&g, nodes);
  graphDef.Clear();
  g.ToGraphDef(&graphDef);
}

void PruneNodesUnReachableTo(GraphDef& graphDef, std::vector<std::string>& nodeNames) {
  Graph g(OpRegistry::Global());
  DlGraphUtil::GraphDef2Graph(graphDef, &g);
  std::unordered_set<const Node*> nodes;
  for (auto& name : nodeNames) {
    nodes.insert(DlGraphUtil::GetNode(g, name));
    nodes.insert(DlGraphUtil::GetNode(g, name));
  }

  PruneNodesUnReachableTo(&g, nodes);
  graphDef.Clear();
  g.ToGraphDef(&graphDef);
}

bool PruneNodesUnReachableTo(Graph* g, std::unordered_set<const Node*> targets) {
  std::deque<const Node*> queue;
  std::set<const Node*> reachable;
  for (const Node* n : targets) {
    VLOG(2) << "Reverse reach init: " << n->name();
    queue.push_back(n);
    reachable.insert(n);
  }
  while (!queue.empty()) {
    const Node* n = queue.front();
    queue.pop_front();
    for (const Node* in : n->in_nodes()) {
      if (reachable.insert(in).second) {
        queue.push_back(in);
        VLOG(2) << "Reverse reach : " << n->name() << " from " << in->name();
      }
    }
  }

  std::vector<Node*> all_nodes;
  all_nodes.reserve(g->num_nodes());
  for (Node* n : g->nodes()) {
    all_nodes.push_back(n);
  }

  bool any_removed = false;
  for (const Node* n : all_nodes) {
    if (reachable.find(n) == reachable.end()) {
      g->RemoveNode(const_cast<Node*>(n));
      any_removed = true;
    }
  }

  return any_removed;
}

void PruneNodesReachableTo(GraphDef& graphDef, std::vector<std::string>& nodeNames) {
  Graph g(OpRegistry::Global());
  DlGraphUtil::GraphDef2Graph(graphDef, &g);
  std::unordered_set<const Node*> nodes;
  for (auto& name : nodeNames) {
    nodes.insert(DlGraphUtil::GetNode(g, name));
    nodes.insert(DlGraphUtil::GetNode(g, name));
  }

  PruneNodesReachableTo(&g, nodes);
  graphDef.Clear();
  g.ToGraphDef(&graphDef);

  for (const Node* node : nodes) {
    const_cast<NodeDef&>(node->def()).clear_input();
  }
}

bool PruneNodesReachableTo(Graph* g, std::unordered_set<const Node*> targets) {
  std::deque<const Node*> queue;
  std::set<const Node*> reachable;
  for (const Node* n : targets) {
    VLOG(2) << "Reverse reach init: " << n->name();
    queue.push_back(n);
  }
  while (!queue.empty()) {
    const Node* n = queue.front();
    queue.pop_front();
    for (const Node* in : n->in_nodes()) {
      if (reachable.insert(in).second) {
        queue.push_back(in);
        VLOG(2) << "Reverse reach : " << n->name() << " from " << in->name();
      }
    }
  }
  std::cout << "nodes reachable to input:" << std::endl;
  for (auto node : reachable)
    std::cout << node->def().DebugString() << std::endl;

  bool any_removed = false;
  for (const Node* n : reachable) {
    g->RemoveNode(const_cast<Node*>(n));
    any_removed = true;
  }

  return any_removed;
}

// TODO:fixme
void PruneNodesUnreachableFrom(GraphDef& graphDef, const std::string& nodeName) {
  Graph g(OpRegistry::Global());
  DlGraphUtil::GraphDef2Graph(graphDef, &g);
  auto* inputNode = const_cast<Node*>(DlGraphUtil::GetNode(g, nodeName));
  DlGraphUtil::PruneNodesUnreachableFrom(g, inputNode);

  graphDef.Clear();
  g.ToGraphDef(&graphDef);
  NodeDef* nodeDef = const_cast<NodeDef*>(GetNode(graphDef, "input"));
  nodeDef->clear_input();

  // DlGraphUtil::WriteToFile(graphDef, "pruned_graph.pbtxt");
}

bool PruneNodesUnreachableFrom(Graph& g, Node* srcNode) {
  std::unordered_set<const Node*> reachableNodes;
  DlGraphUtil::DFS(g, srcNode, [&](Node* node) { reachableNodes.insert(node); }, nullptr);

  std::vector<Node*> all_nodes;
  all_nodes.reserve(g.num_nodes());
  for (Node* n : g.nodes()) {
    all_nodes.push_back(n);
  }

  bool any_removed = false;
  for (Node* n : all_nodes) {
    if (reachableNodes.count(n) == 0/* && !n->IsSource() && !n->IsSink()*/) {
      g.RemoveNode(n);
      any_removed = true;
    }
  }

  return any_removed;
}

void PruneNodesReachableFrom(GraphDef& graphDef, const std::string& nodeName) {
  Graph g(OpRegistry::Global());
  DlGraphUtil::GraphDef2Graph(graphDef, &g);
  auto* inputNode = const_cast<Node*>(DlGraphUtil::GetNode(g, nodeName));
  if (inputNode) {
    DlGraphUtil::PruneNodesReachableFrom(g, inputNode);
    graphDef.Clear();
    g.ToGraphDef(&graphDef);
  }
}

bool PruneNodesReachableFrom(Graph& g, Node* srcNode) {
  std::unordered_set<const Node*> reachableNodes;
  DlGraphUtil::DFS(g, srcNode, [&](Node* node) { reachableNodes.insert(node); }, nullptr);

  bool any_removed = false;
  for (const Node* n : reachableNodes) {
    g.RemoveNode(const_cast<Node*>(n));
    any_removed = true;
  }

  return any_removed;
}

void DFS(const Graph& g, const std::function<void(Node*)>& enter,
         const std::function<void(Node*)>& leave) {
  // Stack of work to do.
  struct Work {
    Node* node;
    bool leave;  // Are we entering or leaving n?
  };
  std::vector<Work> stack;
  stack.push_back(Work{g.source_node(), false});

  std::vector<bool> visited(g.num_node_ids(), false);
  while (!stack.empty()) {
    Work w = stack.back();
    stack.pop_back();

    Node* n = w.node;
    if (w.leave) {
      leave(n);
      continue;
    }

    if (visited[n->id()]) continue;
    visited[n->id()] = true;
    if (enter) enter(n);

    // Arrange to call leave(n) when all done with descendants.
    if (leave) stack.push_back(Work{n, true});

    // Arrange to work on descendants.
    for (Node* out : n->out_nodes()) {
      if (!visited[out->id()]) {
        // Note; we must not mark as visited until we actually process it.
        stack.push_back(Work{out, false});
      }
    }
  }
}

void DFS(const Graph& g, Node* srcNode, const std::function<void(Node*)>& enter,
         const std::function<void(Node*)>& leave) {
  struct Work {
    Node* node;
    bool leave;  // Are we entering or leaving n?
  };
  std::vector<Work> stack;
  stack.push_back(Work{srcNode, false});

  std::vector<bool> visited(g.num_node_ids(), false);
  while (!stack.empty()) {
    Work w = stack.back();
    stack.pop_back();

    Node* n = w.node;
    if (w.leave) {
      leave(n);
      continue;
    }

    if (visited[n->id()]) continue;
    visited[n->id()] = true;
    if (enter) enter(n);

    // Arrange to call leave(n) when all done with descendants.
    if (leave) stack.push_back(Work{n, true});

    // Arrange to work on descendants.
    for (Node* out : n->out_nodes()) {
      if (!visited[out->id()]) {
        // Note; we must not mark as visited until we actually process it.
        stack.push_back(Work{out, false});
      }
    }
  }
}

void GraphDef2Graph(const GraphDef& graphDef, Graph* g_) {
  GraphConstructorOptions opts;
  TF_CHECK_OK(ConvertGraphDefToGraph(opts, graphDef, g_));
}

const ::tensorflow::Node* GetNode(const Graph& graph, const std::string& nodeName) {
  for (auto node : graph.nodes()) {
    if (node->name() == nodeName)
      return node;
  }
  return nullptr;
}

const ::tensorflow::NodeDef* GetNode(const GraphDef& graphDef, const std::string& nodeName) {
  for (int i = 0; i < graphDef.node_size(); i++) {
    if (graphDef.node(i).name() == nodeName) {
      return &graphDef.node(i);
    }
  }
  return nullptr;
}

::tensorflow::NodeDef* GetNode(GraphDef& graphDef, const std::string& nodeName) {
  for (int i = 0; i < graphDef.node_size(); i++) {
    if (graphDef.node(i).name() == nodeName) {
      return graphDef.mutable_node(i);
    }
  }
  return nullptr;
}

void ChangeBatchSize(const GraphDef& graphDef, int matchsize, int _batchsize) {
  GraphDef& nonGraph = const_cast<GraphDef&>(graphDef);
  for (int i = 0; i < graphDef.node_size(); i++) {
    ::tensorflow::NodeDef& nodeDef = const_cast<NodeDef&>(nonGraph.node(i));
    if (nodeDef.name() == "gradients/unit_res_19/sub1/conv1/Conv2D_grad/Conv2DBackpropInput") {
      int x = 9;
      x *= 4;
    }
    ::google::protobuf::Map<::std::string, ::tensorflow::AttrValue>
        & attr = const_cast<::google::protobuf::Map<::std::string, ::tensorflow::AttrValue>&>(nodeDef.attr());
    google::protobuf::Map<::std::string, ::tensorflow::AttrValue>::iterator
        it = attr.begin(); //attr.find("_output_shapes");
    while (it != attr.end()) {
      if (it->first == "_output_shapes") {
        for (int j = 0; j < it->second.list().shape_size(); j++) {
          ::tensorflow::TensorShapeProto& shape = const_cast<::tensorflow::TensorShapeProto&>(it->second.list().shape(
              j));
          if (shape.dim_size() > 0) {
            ::tensorflow::TensorShapeProto_Dim& dim = const_cast<::tensorflow::TensorShapeProto_Dim&>(shape.dim(0));
            if (dim.size() == matchsize)
              dim.set_size(_batchsize);
          }
        }
      }
      ++it;
    }
  }
}

void RemoveNodeWithNamePrefix(const GraphDef& graphDef, const std::string& prefix) {
  ::google::protobuf::RepeatedPtrField<::tensorflow::NodeDef>* nodes = const_cast<GraphDef&>(graphDef).mutable_node();
//  while (it != end) {
//    const ::tensorflow::NodeDef& nodeDef = *it;
//    if (nodeDef.name().find(prefix) == 0) {
//      nodes->erase(it);
//    }
//    ++it;
//  }
  for (int i = graphDef.node_size() - 1; i >= 0; --i) {
    const ::tensorflow::NodeDef& nodeDef = graphDef.node(i);
    if (nodeDef.name().find(prefix) == 0) {
      nodes->DeleteSubrange(i, 1);
    }
  }
}

void RemoveNodeWithName(const GraphDef& graphDef, const std::string& name) {
  ::google::protobuf::RepeatedPtrField<::tensorflow::NodeDef>* nodes = const_cast<GraphDef&>(graphDef).mutable_node();
  for (int i = graphDef.node_size() - 1; i >= 0; --i) {
    const ::tensorflow::NodeDef& nodeDef = graphDef.node(i);
    if (nodeDef.name() == name) {
      nodes->DeleteSubrange(i, 1);
    }
  }
}

void DeleteRepeatedPtrFieldWithPrefix(::google::protobuf::RepeatedPtrField<std::string>& repeatedPtrField,
                                      const std::string& prefix) {
  for (int i = repeatedPtrField.size() - 1; i >= 0; --i) {
    if (repeatedPtrField[i].find(prefix) == 0)
      repeatedPtrField.DeleteSubrange(i, 1);
  }
}

void DeleteRepeatedPtrFieldWithName(::google::protobuf::RepeatedPtrField<std::string>& repeatedPtrField,
                                    const std::string& name) {
  for (int i = repeatedPtrField.size() - 1; i >= 0; --i) {
    if (repeatedPtrField[i] == name)
      repeatedPtrField.DeleteSubrange(i, 1);
  }
}

void ClearNodeInputWithNamePrefix(const GraphDef& graphDef, const std::string& prefix) {
  ::google::protobuf::RepeatedPtrField<::tensorflow::NodeDef>* nodes = const_cast<GraphDef&>(graphDef).mutable_node();
  ::google::protobuf::RepeatedPtrField<::tensorflow::NodeDef>::iterator it = nodes->begin();
  ::google::protobuf::RepeatedPtrField<::tensorflow::NodeDef>::iterator end = nodes->end();

  while (it != end) {
    ::tensorflow::NodeDef& nodeDef = *it;
    DeleteRepeatedPtrFieldWithPrefix(*nodeDef.mutable_input(), prefix);
    ++it;
  }
}

void ClearNodeInputWithName(const GraphDef& graphDef, const std::string& name) {
  ::google::protobuf::RepeatedPtrField<::tensorflow::NodeDef>* nodes = const_cast<GraphDef&>(graphDef).mutable_node();
  ::google::protobuf::RepeatedPtrField<::tensorflow::NodeDef>::iterator it = nodes->begin();
  ::google::protobuf::RepeatedPtrField<::tensorflow::NodeDef>::iterator end = nodes->end();

  while (it != end) {
    ::tensorflow::NodeDef& nodeDef = *it;
    DeleteRepeatedPtrFieldWithName(*nodeDef.mutable_input(), name);
    ++it;
  }
}

void RemoveNodeBefore(const GraphDef& graphDef, const std::string& pivotName) {
  ::google::protobuf::RepeatedPtrField<::tensorflow::NodeDef>* nodes = const_cast<GraphDef&>(graphDef).mutable_node();
  for (int i = 0; i < graphDef.node_size(); i++) {
    const ::tensorflow::NodeDef& nodeDef = graphDef.node(i);
    if (nodeDef.name() == pivotName) {
      nodes->DeleteSubrange(0, i);
      const_cast<NodeDef&>(nodeDef).clear_input();
      break;
    }
  }
}

void WriteToFile(const GraphDef& graphDef, const std::string& filename) {
  std::fstream myfile(filename, std::ios::out);
  // see AddReadControl@tensorflow/tensorflow/core/graph/graph_partition.cc for  input: "^input_variable/Assign"
  const std::string& debug = graphDef.DebugString();
  myfile.write(debug.c_str(), debug.size());
  myfile.close();
}

void PrintGraphNodeInfo(const GraphDef& graphDef) {
  for (int i = 0; i < graphDef.node_size(); i++) {
    const ::tensorflow::NodeDef& nodeDef = graphDef.node(i);
    std::cout << nodeDef.name() << "," << nodeDef.device() << std::endl;
  }
}

void PrintGraphNodeInfo(const GraphDef& graphDef, const std::string& filter) {
  for (int i = 0; i < graphDef.node_size(); i++) {
    const ::tensorflow::NodeDef& nodeDef = graphDef.node(i);
    if (nodeDef.name().find(filter) != std::string::npos)
      std::cout << nodeDef.name() << "," << nodeDef.device() << std::endl;
  }
}

void PrintNodeInfo(const GraphDef& graphDef, const std::string& name) {
  for (int i = 0; i < graphDef.node_size(); i++) {
    const ::tensorflow::NodeDef& nodeDef = graphDef.node(i);
    if (nodeDef.name() == name) {
      std::cout << nodeDef.DebugString() << std::endl;
      break;
    }
  }
}

void ChangeDevice(const GraphDef& graphDef, const std::string& match, const std::string& replacement) {
  for (int i = 0; i < graphDef.node_size(); i++) {
    const tensorflow::NodeDef& nodeDef = graphDef.node(i);
    std::string dev = nodeDef.device();
    string::size_type pos = dev.find(match);
    if (pos != string::npos) {
      dev.replace(pos, match.length(), replacement);
      const_cast<NodeDef&>(nodeDef).set_device(dev);
    }
  }
}

}
