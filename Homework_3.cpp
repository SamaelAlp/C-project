#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <random>
#include <functional>
#include <chrono> 

// Типы ячеек карты
enum class cell_type { Wall, Room, Corridor };

struct Room {
    int x, y, width, height;
    Room(int x, int y, int width, int height) 
        : x(x), y(y), width(width), height(height) {}
    
    struct Point { int x, y; };
    Point center() const {
        return {x + width/2, y + height/2};
    }
};

using Map = std::vector<std::vector<cell_type>>;

Map draw_horizontal(const Map& map, const std::vector<Room>& rooms, int x, int y, int target_x, int target_y);
Map draw_vertical(const Map& map, const std::vector<Room>& rooms, int x, int y, int target_x, int target_y);

bool intersects(const Room& a, const Room& b) {
    return a.x < b.x + b.width &&
           a.x + a.width > b.x &&
           a.y < b.y + b.height &&
           a.y + a.height > b.y;
}

Room generate_random_room(int map_width, int map_height, std::mt19937& rng) {
    std::uniform_int_distribution<int> width_dist(3, 10);
    std::uniform_int_distribution<int> height_dist(3, 10);
    std::uniform_int_distribution<int> x_dist(1, map_width - width_dist(rng) - 1);
    std::uniform_int_distribution<int> y_dist(1, map_height - height_dist(rng) - 1);
    
    int w = width_dist(rng);
    int h = height_dist(rng);
    int x = x_dist(rng);
    int y = y_dist(rng);
    
    return Room(x, y, w, h);
}

std::vector<Room> generate_rooms_helper(
    int n, 
    int width, 
    int height, 
    std::vector<Room> acc, 
    std::mt19937& rng
) {
    if (n == 0) return acc;
    
    Room room = generate_random_room(width, height, rng);
    bool intersects_any = std::any_of(acc.begin(), acc.end(), 
        [&](const Room& r) { return intersects(room, r); });
    
    if (intersects_any) {
        return generate_rooms_helper(n, width, height, acc, rng);
    } else {
        acc.push_back(room);
        return generate_rooms_helper(n - 1, width, height, std::move(acc), rng);
    }
}

Map create_map(int width, int height, const std::vector<Room>& rooms) {
    Map map(height, std::vector<cell_type>(width, cell_type::Wall));
    
    for (const auto& room : rooms) {
        for (int y = room.y; y < room.y + room.height; ++y) {
            for (int x = room.x; x < room.x + room.width; ++x) {
                map[y][x] = cell_type::Room;
            }
        }
    }
    return map;
}

struct Edge {
    int u, v;
    int weight;
    Edge(int u, int v, int weight) : u(u), v(v), weight(weight) {}
};

std::vector<Edge> generate_edges(const std::vector<Room::Point>& centers) {
    std::vector<Edge> edges;
    for (int i = 0; i < centers.size(); ++i) {
        for (int j = i + 1; j < centers.size(); ++j) {
            int dx = centers[i].x - centers[j].x;
            int dy = centers[i].y - centers[j].y;
            edges.emplace_back(i, j, dx*dx + dy*dy);
        }
    }
    return edges;
}

std::vector<std::set<int>> initial_components(int n) {
    std::vector<std::set<int>> components;
    for (int i = 0; i < n; ++i) {
        components.push_back({i});
    }
    return components;
}

std::vector<Edge> kruskal_helper(
    const std::vector<Edge>& edges,
    const std::vector<std::set<int>>& components,
    const std::vector<Edge>& mst
) {
    if (edges.empty()) return mst;
    
    Edge edge = edges[0];
    std::vector<Edge> remaining(edges.begin() + 1, edges.end());
    
    auto comp_u = std::find_if(components.begin(), components.end(),
        [&](const std::set<int>& c) { return c.count(edge.u); });
    auto comp_v = std::find_if(components.begin(), components.end(),
        [&](const std::set<int>& c) { return c.count(edge.v); });
    
    if (comp_u != components.end() && comp_v != components.end() && comp_u != comp_v) {
        std::set<int> new_comp = *comp_u;
        new_comp.insert(comp_v->begin(), comp_v->end());
        
        std::vector<std::set<int>> new_components;
        for (const auto& c : components) {
            if (&c != &*comp_u && &c != &*comp_v) {
                new_components.push_back(c);
            }
        }
        new_components.push_back(new_comp);
        
        std::vector<Edge> new_mst = mst;
        new_mst.push_back(edge);
        
        return kruskal_helper(remaining, new_components, new_mst);
    }
    
    return kruskal_helper(remaining, components, mst);
}

Map draw_horizontal(
    const Map& map,
    const std::vector<Room>& rooms,
    int x, int y,
    int target_x, int target_y
) {
    if (x == target_x) {
        return draw_vertical(map, rooms, x, y, target_x, target_y);
    }
    
    int dx = (target_x > x) ? 1 : -1;
    Map new_map = map;
    
    bool in_room = std::any_of(rooms.begin(), rooms.end(), 
        [x, y](const Room& r) {
            return x >= r.x && x < r.x + r.width &&
                   y >= r.y && y < r.y + r.height;
        });
    
    if (!in_room) {
        new_map[y][x] = cell_type::Corridor;
    }
    
    return draw_horizontal(new_map, rooms, x + dx, y, target_x, target_y);
}

Map draw_vertical(
    const Map& map,
    const std::vector<Room>& rooms,
    int x, int y,
    int target_x, int target_y
) {
    if (y == target_y) return map;
    
    int dy = (target_y > y) ? 1 : -1;
    Map new_map = map;
    
    bool in_room = std::any_of(rooms.begin(), rooms.end(), 
        [x, y](const Room& r) {
            return x >= r.x && x < r.x + r.width &&
                   y >= r.y && y < r.y + r.height;
        });
    
    if (!in_room) {
        new_map[y][x] = cell_type::Corridor;
    }
    
    return draw_vertical(new_map, rooms, x, y + dy, target_x, target_y);
}

Map add_corridors(
    const Map& map,
    const std::vector<Room>& rooms,
    const std::vector<Edge>& mst
) {
    if (mst.empty()) return map;
    
    const Edge& edge = mst[0];
    auto start = rooms[edge.u].center();
    auto end = rooms[edge.v].center();
    
    Map new_map = draw_horizontal(map, rooms, start.x, start.y, end.x, end.y);
    std::vector<Edge> remaining(mst.begin() + 1, mst.end());
    
    return add_corridors(new_map, rooms, remaining);
}

std::string render_map(const Map& map) {
    std::string result;
    for (const auto& row : map) {
        for (cell_type cell : row) {
            result += (cell == cell_type::Wall) ? '#' : '.';
        }
        result += '\n';
    }
    return result;
}

int main() {
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(seed);
    
    const int width = 80;
    const int height = 25;
    const int num_rooms = 5;
    
    auto rooms = generate_rooms_helper(num_rooms, width, height, {}, rng);
    
    Map map = create_map(width, height, rooms);
    
    std::vector<Room::Point> centers;
    for (const auto& room : rooms) {
        centers.push_back(room.center());
    }
    
    auto edges = generate_edges(centers);
    std::sort(edges.begin(), edges.end(), 
        [](const Edge& a, const Edge& b) { return a.weight < b.weight; });
    
    auto components = initial_components(centers.size());
    auto mst = kruskal_helper(edges, components, {});
    
    Map final_map = add_corridors(map, rooms, mst);
    
    std::cout << render_map(final_map);
    
    return 0;
}