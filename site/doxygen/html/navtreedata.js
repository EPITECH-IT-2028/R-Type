/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "R-Type", "index.html", [
    [ "🧩 ECS Development Guide", "md_docs_2server__how__to.html", null ],
    [ "R-TYPE NETWORK PROTOCOL SPECIFICATION", "md_docs_2server__protocol.html", [
      [ "Abstract", "md_docs_2server__protocol.html#autotoc_md8", null ],
      [ "Status of This Memo", "md_docs_2server__protocol.html#autotoc_md10", null ],
      [ "Copyright Notice", "md_docs_2server__protocol.html#autotoc_md12", null ],
      [ "Table of Contents", "md_docs_2server__protocol.html#autotoc_md14", null ],
      [ "1. Introduction", "md_docs_2server__protocol.html#autotoc_md16", null ],
      [ "2. Terminology and Conventions", "md_docs_2server__protocol.html#autotoc_md18", null ],
      [ "3. Protocol Overview", "md_docs_2server__protocol.html#autotoc_md20", null ],
      [ "4. Message Header", "md_docs_2server__protocol.html#autotoc_md22", null ],
      [ "5. Message Types", "md_docs_2server__protocol.html#autotoc_md24", [
        [ "PacketType Enumeration", "md_docs_2server__protocol.html#autotoc_md25", null ],
        [ "5.1. Client-to-Server Messages", "md_docs_2server__protocol.html#autotoc_md27", [
          [ "PlayerInfo (0x04)", "md_docs_2server__protocol.html#autotoc_md28", null ],
          [ "Position (0x05)", "md_docs_2server__protocol.html#autotoc_md29", null ],
          [ "PlayerShoot (0x09)", "md_docs_2server__protocol.html#autotoc_md30", null ],
          [ "PlayerHit (0x12)", "md_docs_2server__protocol.html#autotoc_md31", null ],
          [ "PlayerDisconnect (0x0F)", "md_docs_2server__protocol.html#autotoc_md32", null ],
          [ "Heartbeat (0x10)", "md_docs_2server__protocol.html#autotoc_md33", null ]
        ] ],
        [ "5.2. Server-to-Client Messages", "md_docs_2server__protocol.html#autotoc_md35", [
          [ "NewPlayer (0x03)", "md_docs_2server__protocol.html#autotoc_md36", null ],
          [ "Move (0x02)", "md_docs_2server__protocol.html#autotoc_md37", null ],
          [ "EnemySpawn (0x06)", "md_docs_2server__protocol.html#autotoc_md38", null ],
          [ "EnemyMove (0x07)", "md_docs_2server__protocol.html#autotoc_md39", null ],
          [ "EnemyDeath (0x08)", "md_docs_2server__protocol.html#autotoc_md40", null ],
          [ "EnemyHit (0x11)", "md_docs_2server__protocol.html#autotoc_md41", null ],
          [ "ProjectileSpawn (0x0A)", "md_docs_2server__protocol.html#autotoc_md42", null ],
          [ "ProjectileHit (0x0B)", "md_docs_2server__protocol.html#autotoc_md43", null ],
          [ "ProjectileDestroy (0x0C)", "md_docs_2server__protocol.html#autotoc_md44", null ],
          [ "PlayerDeath (0x13)", "md_docs_2server__protocol.html#autotoc_md45", null ],
          [ "GameStart (0x0D)", "md_docs_2server__protocol.html#autotoc_md46", null ],
          [ "GameEnd (0x0E)", "md_docs_2server__protocol.html#autotoc_md47", null ]
        ] ]
      ] ],
      [ "6. Data Types", "md_docs_2server__protocol.html#autotoc_md49", null ],
      [ "7. Message Semantics", "md_docs_2server__protocol.html#autotoc_md51", null ],
      [ "8. Example Exchange", "md_docs_2server__protocol.html#autotoc_md53", null ],
      [ "9. Security Considerations", "md_docs_2server__protocol.html#autotoc_md55", null ],
      [ "10. Future Work", "md_docs_2server__protocol.html#autotoc_md57", null ],
      [ "Appendix A. Packet Layouts", "md_docs_2server__protocol.html#autotoc_md59", null ],
      [ "Authors", "md_docs_2server__protocol.html#autotoc_md61", null ]
    ] ],
    [ "Asset Management", "md_docs_2wiki_2_asset_management.html", [
      [ "Overview", "md_docs_2wiki_2_asset_management.html#autotoc_md63", null ],
      [ "<span class=\"tt\">AssetManager</span> (<span class=\"tt\">client/AssetManager.hpp</span>)", "md_docs_2wiki_2_asset_management.html#autotoc_md64", [
        [ "Key Methods:", "md_docs_2wiki_2_asset_management.html#autotoc_md65", null ]
      ] ],
      [ "<span class=\"tt\">RenderManager</span> (<span class=\"tt\">client/RenderManager.hpp</span>)", "md_docs_2wiki_2_asset_management.html#autotoc_md66", [
        [ "Asset Paths Defined:", "md_docs_2wiki_2_asset_management.html#autotoc_md67", null ]
      ] ],
      [ "Asset Resources (<span class=\"tt\">client/resources/</span>)", "md_docs_2wiki_2_asset_management.html#autotoc_md68", [
        [ "Contents:", "md_docs_2wiki_2_asset_management.html#autotoc_md69", null ]
      ] ]
    ] ],
    [ "Contribution Guidelines", "md_docs_2wiki_2_contribution_guidelines.html", [
      [ "How to Contribute", "md_docs_2wiki_2_contribution_guidelines.html#autotoc_md71", null ],
      [ "Coding Style and Conventions", "md_docs_2wiki_2_contribution_guidelines.html#autotoc_md72", null ],
      [ "Testing", "md_docs_2wiki_2_contribution_guidelines.html#autotoc_md73", null ],
      [ "Pull Request Guidelines", "md_docs_2wiki_2_contribution_guidelines.html#autotoc_md74", null ],
      [ "Reporting Bugs", "md_docs_2wiki_2_contribution_guidelines.html#autotoc_md75", null ]
    ] ],
    [ "Game Engine (ECS)", "md_docs_2wiki_2_game_engine_e_c_s.html", [
      [ "Core Concepts", "md_docs_2wiki_2_game_engine_e_c_s.html#autotoc_md77", null ],
      [ "Architecture Overview", "md_docs_2wiki_2_game_engine_e_c_s.html#autotoc_md78", [
        [ "<span class=\"tt\">ECSManager</span>", "md_docs_2wiki_2_game_engine_e_c_s.html#autotoc_md79", null ],
        [ "<span class=\"tt\">EntityManager</span>", "md_docs_2wiki_2_game_engine_e_c_s.html#autotoc_md80", null ],
        [ "<span class=\"tt\">ComponentManager</span>", "md_docs_2wiki_2_game_engine_e_c_s.html#autotoc_md81", null ],
        [ "<span class=\"tt\">SystemManager</span>", "md_docs_2wiki_2_game_engine_e_c_s.html#autotoc_md82", null ],
        [ "<span class=\"tt\">System</span> Base Class", "md_docs_2wiki_2_game_engine_e_c_s.html#autotoc_md83", null ]
      ] ],
      [ "How it Works Together", "md_docs_2wiki_2_game_engine_e_c_s.html#autotoc_md84", null ]
    ] ],
    [ "Getting Started", "md_docs_2wiki_2_getting_started.html", [
      [ "Prerequisites", "md_docs_2wiki_2_getting_started.html#autotoc_md86", null ],
      [ "Building the Project", "md_docs_2wiki_2_getting_started.html#autotoc_md87", null ],
      [ "Running the Server", "md_docs_2wiki_2_getting_started.html#autotoc_md88", null ],
      [ "Running the Client", "md_docs_2wiki_2_getting_started.html#autotoc_md89", null ]
    ] ],
    [ "R-Type Project", "md_docs_2wiki_2_home.html", [
      [ "Overview", "md_docs_2wiki_2_home.html#autotoc_md91", null ],
      [ "Features", "md_docs_2wiki_2_home.html#autotoc_md92", null ],
      [ "Project Structure", "md_docs_2wiki_2_home.html#autotoc_md93", null ],
      [ "Getting Started", "md_docs_2wiki_2_home.html#autotoc_md94", null ],
      [ "Contributing", "md_docs_2wiki_2_home.html#autotoc_md95", null ],
      [ "Technical Details", "md_docs_2wiki_2_home.html#autotoc_md96", null ]
    ] ],
    [ "Networking", "md_docs_2wiki_2_networking.html", [
      [ "Overview", "md_docs_2wiki_2_networking.html#autotoc_md98", null ],
      [ "Core Components", "md_docs_2wiki_2_networking.html#autotoc_md99", [
        [ "<span class=\"tt\">BaseNetworkManager</span>", "md_docs_2wiki_2_networking.html#autotoc_md100", null ],
        [ "<span class=\"tt\">ClientNetworkManager</span>", "md_docs_2wiki_2_networking.html#autotoc_md101", null ],
        [ "<span class=\"tt\">ServerNetworkManager</span>", "md_docs_2wiki_2_networking.html#autotoc_md102", null ],
        [ "<span class=\"tt\">Packet.hpp</span>", "md_docs_2wiki_2_networking.html#autotoc_md103", null ]
      ] ],
      [ "R-Type Network Protocol (RTNP)", "md_docs_2wiki_2_networking.html#autotoc_md104", null ],
      [ "Communication Flow Example", "md_docs_2wiki_2_networking.html#autotoc_md105", null ]
    ] ],
    [ "Project Structure", "md_docs_2wiki_2_project_structure.html", [
      [ "Main Directories Explained", "md_docs_2wiki_2_project_structure.html#autotoc_md107", null ]
    ] ],
    [ "R-Type - A Game Engine That Roars! 🚀", "md__r_e_a_d_m_e.html", [
      [ "📖 Overview", "md__r_e_a_d_m_e.html#autotoc_md109", [
        [ "Key Features", "md__r_e_a_d_m_e.html#autotoc_md110", null ]
      ] ],
      [ "🚀 Quick Start", "md__r_e_a_d_m_e.html#autotoc_md111", [
        [ "Prerequisites", "md__r_e_a_d_m_e.html#autotoc_md112", null ],
        [ "Installation", "md__r_e_a_d_m_e.html#autotoc_md113", null ]
      ] ],
      [ "🏗️ Architecture", "md__r_e_a_d_m_e.html#autotoc_md114", [
        [ "Project Structure", "md__r_e_a_d_m_e.html#autotoc_md115", null ],
        [ "Design Patterns", "md__r_e_a_d_m_e.html#autotoc_md116", null ]
      ] ],
      [ "📦 Dependencies", "md__r_e_a_d_m_e.html#autotoc_md117", null ],
      [ "👥 Team", "md__r_e_a_d_m_e.html#autotoc_md118", null ],
      [ "🙏 Acknowledgments", "md__r_e_a_d_m_e.html#autotoc_md119", null ]
    ] ],
    [ "R-Type Build System", "md__r_e_a_d_m_e___b_u_i_l_d.html", [
      [ "Prerequisites", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md122", null ],
      [ "Quick Start", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md123", null ],
      [ "Build Script Usage", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md124", [
        [ "No arguments (default)", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md125", null ],
        [ "Single argument - Build Type", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md126", null ],
        [ "Single argument - Target", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md127", null ],
        [ "Single argument - Clean", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md128", null ],
        [ "Two arguments - Any order", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md129", null ],
        [ "Manual build:", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md130", [
          [ "Building the Client", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md131", null ],
          [ "Building the Server", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md132", null ],
          [ "Building Both", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md133", null ]
        ] ]
      ] ],
      [ "Dependencies", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md134", null ],
      [ "Output", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md135", null ],
      [ "Debug Builds", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md136", null ],
      [ "CMake Options", "md__r_e_a_d_m_e___b_u_i_l_d.html#autotoc_md137", null ]
    ] ],
    [ "R-Type - Technical Comparative Study", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html", [
      [ "📋 Executive Summary", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md139", null ],
      [ "🔧 Programming Language Choice", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md141", [
        [ "C++20 vs Alternatives", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md142", null ]
      ] ],
      [ "🎨 Graphics Library Analysis", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md144", [
        [ "Raylib vs Alternatives", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md145", null ]
      ] ],
      [ "🌐 Networking Technology", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md147", [
        [ "ASIO vs Alternatives", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md148", null ],
        [ "Protocol Choice: UDP vs TCP", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md149", null ]
      ] ],
      [ "🏗️ Architecture and Data Structures", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md151", [
        [ "Entity-Component-System (ECS)", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md152", null ],
        [ "Network Data Structures", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md153", null ]
      ] ],
      [ "💾 Storage and Configuration", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md155", [
        [ "Configuration Format", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md156", null ],
        [ "Memory Management", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md157", null ]
      ] ],
      [ "🔒 Security Analysis", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md159", [
        [ "Current Security Measures", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md160", [
          [ "Network Security", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md161", null ],
          [ "Anti-Cheat Implementation", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md162", null ]
        ] ]
      ] ],
      [ "🚀 Build System", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md164", [
        [ "CMake + Conan", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md165", null ]
      ] ],
      [ "📊 Performance Considerations", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md167", [
        [ "Benchmarks", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md168", null ]
      ] ],
      [ "🔮 Future Improvements", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md170", [
        [ "Short-term", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md171", null ],
        [ "Long-term", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md172", null ]
      ] ],
      [ "📝 Conclusion", "md__t_e_c_h_n_i_c_a_l___c_o_m_p_a_r_a_t_i_v_e___s_t_u_d_y.html#autotoc_md174", null ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", "functions_vars" ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"_asset_manager_8cpp.html",
"classecs_1_1_background_system.html#a34aeedf30ee12c1e47c00aed59a65d65",
"classgame_1_1_enemy.html#a80203095c78b70cc202b6e40fbf042f4",
"classpacket_1_1_enemy_move_handler.html#adff9c0d290fa12359ee8afc6cabd7332",
"functions_x.html",
"struct_game_end_packet.html#afee93b6c4410d96c4112e6e0c9799723",
"structqueue_1_1_enemy_hit_event.html"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';
var LISTOFALLMEMBERS = 'List of all members';