# Run the build system with `--graph` to generate `dependency_graph.dot` first.
dot -Tsvg dependency_graph.dot -o dependency_graph.svg -Gsplines=true -Gratio=1 -Earrowhead=diamond -Earrowtail=odiamond -Edir=both -Nshape=diamond -K fdp
