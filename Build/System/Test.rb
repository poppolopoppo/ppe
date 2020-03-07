$Build.library!(:BuildGraph) do
    define!('PPE_BUILDGRAPH_VERSION=1.0.0')
    public_deps!(
        *namespace[:Runtime]{[ self.Core, self.VFS ]},
        namespace[:Runtime, :Graphics].Test )
end
