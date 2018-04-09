
#ifndef TFRECORD_GONET_DLGRAPHUTIL_H
#define TFRECORD_GONET_DLGRAPHUTIL_H

using namespace tensorflow;

namespace DlGraphUtil {
void ClearInput(GraphDef& graphDef, const std::string& nodeName);
void OptimizeGraph(GraphDef& graphDef, std::vector<std::string>& outNodeNames, const std::string& inputName);
void PruneNodesUnReachableTo(GraphDef& graphDef, std::vector<std::string>& nodeNames);
bool PruneNodesUnReachableTo(Graph* g, std::unordered_set<const Node*> targets);
void PruneNodesReachableTo(GraphDef& graphDef, std::vector<std::string>& nodeNames);
bool PruneNodesReachableTo(Graph* g, std::unordered_set<const Node*> targets);
void PruneForReverseReachability(GraphDef& graphDef, std::vector<std::string>& nodeNames);
void PruneNodesUnreachableFrom(GraphDef& graphDef, const std::string& nodeName);
bool PruneNodesUnreachableFrom(Graph& g, Node* srcNode);
void PruneNodesReachableFrom(GraphDef& graphDef, const std::string& nodeName);
bool PruneNodesReachableFrom(Graph& g, Node* srcNode);
void DFS(const Graph& g, const std::function<void(Node*)>& enter,
         const std::function<void(Node*)>& leave);
void DFS(const Graph& g, Node* srcNode, const std::function<void(Node*)>& enter,
         const std::function<void(Node*)>& leave);
void GraphDef2Graph(const GraphDef& graphDef, Graph* g_);
const ::tensorflow::Node* GetNode(const Graph& graph, const std::string& nodeName);
const ::tensorflow::NodeDef* GetNode(const GraphDef& graphDef, const std::string& nodeName);
::tensorflow::NodeDef* GetNode(GraphDef& graphDef, const std::string& nodeName);
void ChangeBatchSize(const GraphDef& graphDef, int matchsize, int _batchsize);
void RemoveNodeWithNamePrefix(const GraphDef& graphDef, const std::string& prefix);
void RemoveNodeWithName(const GraphDef& graphDef, const std::string& name);
void ClearNodeInputWithNamePrefix(const GraphDef& graphDef, const std::string& prefix);
void ClearNodeInputWithName(const GraphDef& graphDef, const std::string& name);
void RemoveNodeBefore(const GraphDef& graphDef, const std::string& pivotName);
void WriteToFile(const GraphDef& graphDef, const std::string& filename);
void PrintGraphNodeInfo(const GraphDef& graphDef);
void PrintGraphNodeInfo(const GraphDef& graphDef, const std::string& filter);
void PrintNodeInfo(const GraphDef& graphDef, const std::string& name);
void ChangeDevice(const GraphDef& graphDef, const std::string& match, const std::string& replacement);
}

#endif //TFRECORD_GONET_DLGRAPHUTIL_H
