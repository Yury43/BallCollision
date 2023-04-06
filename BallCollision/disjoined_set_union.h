#pragma once
#include <vector>


class DisjoinedSetUnion
{
public:

    explicit DisjoinedSetUnion(const int n)
    {
        parent.resize(n);
        size.resize(n);

        for (int v = 0; v < n; ++v)
        {
            MakeSet(v);
        }
    }

    int find_set(int const v)
    {
        if (v == parent[v])
            return v;
        return parent[v] = find_set(parent[v]);
    }

    void join(int a, int b)
    {
        a = find_set(a);
        b = find_set(b);

        if (a != b)
        {
            if (size[a] < size[b])
                std::swap(a, b);

            parent[b] = a;

            size[a] += size[b];
        }
    }

    int get_set_size(int const v)
    {
        return size[find_set(v)];
    }

    int count_sets() const 
    {
        int cnt = 0;
        for (int v = 0; v < parent.size(); ++v)
        {
            if (parent[v] == v)
                cnt++;
        }

        return cnt;
    }

    std::unordered_map<int, std::vector<int>> sets_by_parent()
    {
        std::unordered_map<int, std::vector<int>> labeledGroups;

        for (int v = 0; v < parent.size(); ++v)
        {
            int p = find_set(v);
            if (labeledGroups.find(p) != labeledGroups.end())
            {
                labeledGroups[p].push_back(v);
            }
            else
            {
                labeledGroups[p] = { v };
            }
        }

        return labeledGroups;
    }

private:

    std::vector<int> parent;
    std::vector<int> size;

    void MakeSet(const int v)
    {
        parent[v] = v;
        size[v] = 1;
    }
};
