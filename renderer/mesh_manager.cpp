/* Copyright (c) 2017-2019 Hans-Kristian Arntzen
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "mesh_manager.hpp"
#include "gltf.hpp"
#include "mesh_util.hpp"
#include <stdexcept>

using namespace std;

namespace Granite
{
MeshManager::MeshGroup *MeshManager::register_mesh(const std::string &path)
{
	Util::Hasher hash;
	hash.string(path);

	MeshGroup *group = groups.find(hash.get());
	if (group)
		return group;

	group = groups.emplace_yield(hash.get());

	GLTF::Parser parser(path);
	if (parser.get_scenes().empty())
		throw logic_error("No scenes in glTF.");

	auto &scene = parser.get_scenes().front();
	group->top_level_nodes = scene.node_indices;
	group->node_hierarchy = parser.get_nodes();
	for (auto &mesh : parser.get_meshes())
		group->renderables.push_back(create_imported_mesh(mesh, parser.get_materials().data()));

	return group;
}

static std::vector<Scene::NodeHandle> create_nodes(Scene &scene, const std::vector<Granite::SceneFormats::Node> &nodes)
{
	vector<Scene::NodeHandle> scene_nodes(nodes.size());

	for (size_t i = 0; i < nodes.size(); i++)
	{
		auto &node_info = nodes[i];
		if (node_info.has_skin)
			throw logic_error("Skinning not yet supported for MeshManager.");

		auto node = scene.create_node();
		scene_nodes[i] = move(node);
	}

	for (size_t i = 0; i < nodes.size(); i++)
	{
		auto &node_info = nodes[i];
		for (auto &child : node_info.children)
			scene_nodes[i]->add_child(scene_nodes[child]);
	}

	return scene_nodes;
}

static Scene::NodeHandle create_root_node(Scene &scene, const std::vector<Scene::NodeHandle> &nodes, const std::vector<uint32_t> &top_level_nodes)
{
	auto root = scene.create_node();
	for (auto &top_level : top_level_nodes)
		root->add_child(nodes[top_level]);
	return root;
}

MeshManager::SingleHandle MeshManager::instantiate_renderable(Scene &scene, MeshGroup *group)
{
	auto nodes = create_nodes(scene, group->node_hierarchy);

	SingleHandle handles = {};
	handles.root_node = create_root_node(scene, nodes, group->top_level_nodes);

	for (auto &node : group->node_hierarchy)
	{
		if (!node.meshes.empty())
		{
			unsigned mesh_index = node.meshes.front();
			auto node_index = unsigned(&node - group->node_hierarchy.data());
			handles.entity = scene.create_renderable(group->renderables[mesh_index], nodes[node_index].get());
			break;
		}
	}

	return handles;
}

MeshManager::MultiHandle MeshManager::instantiate_renderables(Scene &scene, MeshGroup *group)
{
	auto nodes = create_nodes(scene, group->node_hierarchy);

	MultiHandle handles = {};
	handles.root_node = create_root_node(scene, nodes, group->top_level_nodes);

	for (auto &node : group->node_hierarchy)
	{
		for (auto &mesh_index : node.meshes)
		{
			auto node_index = unsigned(&node - group->node_hierarchy.data());
			handles.entities.push_back(scene.create_renderable(group->renderables[mesh_index], nodes[node_index].get()));
		}
	}

	return handles;
}
}