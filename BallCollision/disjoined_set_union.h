#pragma once
#include <vector>


class DisjoinedSetUnion
{
public:
    DisjoinedSetUnion(int n)
    {
        parent.resize(n);
        size.resize(n);

        for (int v = 0; v < n; ++v)
        {
            MakeSet(v);
        }
    }

    int FindSet(int v)
    {
        if (v == parent[v])
            return v;
        return parent[v] = FindSet(parent[v]);
    }

    void Join(int a, int b)
    {
        a = FindSet(a);
        b = FindSet(b);

        if (a != b)
        {
            if (size[a] < size[b])
                std::swap(a, b);

            parent[b] = a;

            size[a] += size[b];
        }
    }

    int GetSetSize(int v)
    {
        return size[FindSet(v)];
    }

    int CountSets()
    {
        int cnt = 0;
        for (int v = 0; v < parent.size(); ++v)
        {
            if (parent[v] == v)
                cnt++;
        }

        return cnt;
    }

    std::unordered_map<int, std::vector<int>> SetsByParent()
    {
        std::unordered_map<int, std::vector<int>> labeledGroups;

        for (int v = 0; v < parent.size(); ++v)
        {
            int p = FindSet(v);
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

    void MakeSet(int v)
    {
        parent[v] = v;
        size[v] = 1;
    }
};
