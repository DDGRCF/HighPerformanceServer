let s:cpo_save=&cpo
set cpo&vim
inoremap <silent> <expr> <Plug>(neosnippet_start_unite_snippet) unite#sources#neosnippet#start_complete()
inoremap <silent> <expr> <Plug>(neosnippet_jump) neosnippet#mappings#jump_impl()
inoremap <silent> <expr> <Plug>(neosnippet_expand) neosnippet#mappings#expand_impl()
inoremap <silent> <expr> <Plug>(neosnippet_jump_or_expand) neosnippet#mappings#jump_or_expand_impl()
inoremap <silent> <expr> <Plug>(neosnippet_expand_or_jump) neosnippet#mappings#expand_or_jump_impl()
inoremap <silent> <expr> <Plug>(coc-snippets-expand-jump) coc#_insert_key('request', 'coc-snippets-expand-jump', 1)
inoremap <silent> <expr> <Plug>(coc-snippets-expand) coc#_insert_key('request', 'coc-snippets-expand', 1)
inoremap <nowait> <silent> <expr> <C-B> coc#float#has_scroll() ? "\=coc#float#scroll(0)\" : "\<Left>"
inoremap <nowait> <silent> <expr> <C-F> coc#float#has_scroll() ? "\=coc#float#scroll(1)\" : "\<Right>"
inoremap <silent> <expr> <C-Space> coc#refresh()
inoremap <silent> <Plug>NERDCommenterInsert  <BS>:call NERDComment('i', 'insert')
imap <C-G>S <Plug>ISurround
imap <C-G>s <Plug>Isurround
imap <C-S> <Plug>Isurround
inoremap <silent> <Plug>(table-mode-tableize) |:call tablemode#TableizeInsertMode()a
inoremap <silent> <F25> :silent doautocmd <nomodeline> FocusGained %
inoremap <silent> <F24> :silent doautocmd <nomodeline> FocusLost %
inoremap <silent> <expr> <Plug>delimitMateS-BS delimitMate#WithinEmptyPair() ? "\<Del>" : "\<S-BS>"
inoremap <silent> <Plug>delimitMateBS =delimitMate#BS()
inoremap <silent> <Plug>(complete_parameter#overload_up) :call cmp#overload_next(0)
inoremap <silent> <Plug>(complete_parameter#overload_down) :call cmp#overload_next(1)
inoremap <silent> <Plug>(complete_parameter#goto_previous_parameter) :call cmp#goto_next_param(0)
inoremap <silent> <Plug>(complete_parameter#goto_next_parameter) :call cmp#goto_next_param(1)
imap <silent> <C-G>% <Plug>(matchup-c_g%)
inoremap <silent> <Plug>(matchup-c_g%) :call matchup#motion#insert_mode()
inoremap <silent> <expr> <C-Y> coc#pum#visible() ? coc#pum#confirm() : "\"
inoremap <silent> <expr> <C-E> coc#pum#visible() ? coc#pum#cancel() : "\"
inoremap <silent> <expr> <C-P> coc#pum#visible() ? coc#pum#prev(1) : "\"
inoremap <silent> <expr> <C-N> coc#pum#visible() ? coc#pum#next(1) : "\"
imap <expr> <M-/> neosnippet#expandable() ? "\<Plug>(neosnippet_expand)" : ""
inoremap <expr> <PageUp> pumvisible() ? "\<PageUp>\\" : "\<PageUp>"
inoremap <expr> <PageDown> pumvisible() ? "\<PageDown>\\" : "\<PageDown>"
inoremap <expr> <Up> pumvisible() ? "\" : "\<Up>"
inoremap <expr> <Down> pumvisible() ? "\" : "\<Down>"
inoremap <expr> <S-Tab> coc#pum#visible() ? coc#pum#prev(1) : "\"
cnoremap <expr> <C-K> repeat('<Del>', strchars(getcmdline()) - getcmdpos() + 1)
cnoremap <C-B> <Left>
cnoremap <C-A> <Home>
cnoremap <C-F> <Right>
cnoremap <C-S> w
inoremap <S-CR> o
inoremap <silent> <C-S-Up> :m .-2==gi
inoremap <silent> <C-S-Down> :m .+1==gi
imap <C-W> <Plug>(coc-snippets-expand-jump)
inoremap <C-U> u
vnoremap <nowait> <silent> <expr>  coc#float#has_scroll() ? coc#float#scroll(0) : "\"
nnoremap <nowait> <silent> <expr>  coc#float#has_scroll() ? coc#float#scroll(0) : "\"
omap  <Plug>(SmoothieBackwards)
map  <Plug>(SmoothieDownwards)
vnoremap <nowait> <silent> <expr>  coc#float#has_scroll() ? coc#float#scroll(1) : "\"
nnoremap <nowait> <silent> <expr>  coc#float#has_scroll() ? coc#float#scroll(1) : "\"
omap  <Plug>(SmoothieForwards)
nnoremap <silent>  :call SpaceVim#plugins#ctrlg#display()
nnoremap <silent>  :TmuxNavigateLeft
smap <expr> 	 neosnippet#expandable_or_jumpable() ? "\<Plug>(neosnippet_expand_or_jump)" : (complete_parameter#jumpable(1) ? "\<Plug>(complete_parameter#goto_next_parameter)" : "\	")
xnoremap 	 >gv
nnoremap <silent> <NL> :TmuxNavigateDown
nnoremap <silent>  :TmuxNavigateUp
nnoremap <silent>  :TmuxNavigateRight
nnoremap <silent>  :Unite file_rec/neovim
vnoremap  :w
nnoremap  :w
map  <Plug>(SmoothieUpwards)
nnoremap <nowait> <silent>  a :CocList diagnostics
nmap  xd  [SPC]xd[SPC]
nmap  xa  [SPC]xa[SPC]
vmap   [SPC]
nmap   [SPC]
xnoremap # y?\V"
omap <silent> % <Plug>(matchup-%)
xmap <silent> % <Plug>(matchup-%)
nmap <silent> % <Plug>(matchup-%)
nnoremap & :&&
xnoremap * y/\V"
xmap , [SPC]l
nmap , [SPC]l
nnoremap <silent> ,  :silent! keeppatterns %substitute/\s\+$//e
nnoremap < <<_
xnoremap < <gv
nnoremap > >>_
xnoremap > >gv
omap F <Plug>(clever-f-F)
xmap F <Plug>(clever-f-F)
nmap F <Plug>(clever-f-F)
vmap <silent> J <Plug>(jplus)
nmap <silent> J <Plug>(jplus)
nnoremap <silent> K :call ShowDocumentation()
xmap S <Plug>VSurround
omap T <Plug>(clever-f-T)
xmap T <Plug>(clever-f-T)
nmap T <Plug>(clever-f-T)
xmap V <Plug>(expand_region_shrink)
nnoremap Y y$
nmap <silent> [g <Plug>(coc-diagnostic-prev)
nnoremap <silent> [SPC]ao :lua require("spacevim.plugin.todo").list()
omap <silent> [% <Plug>(matchup-[%)
xmap <silent> [% <Plug>(matchup-[%)
nmap <silent> [% <Plug>(matchup-[%)
nnoremap <silent> [SPC]as :Startify | doautocmd WinEnter
xmap <silent> [SPC]de <Plug>VimspectorBalloonEval
nmap <silent> [SPC]de <Plug>VimspectorBalloonEval
nnoremap <silent> [SPC]dk :call vimspector#Stop() | VimspectorReset
nnoremap <silent> [SPC]dd :call vimspector#DownFrame()
nnoremap <silent> [SPC]du :call vimspector#UpFrame()
nnoremap <silent> [SPC]dO :call vimspector#StepOut()
nnoremap <silent> [SPC]di :call vimspector#StepInto()
nnoremap <silent> [SPC]do :call vimspector#StepOver()
nnoremap <silent> [SPC]dB :call vimspector#ClearBreakpoints()
nnoremap <silent> [SPC]db :call vimspector#ToggleBreakpoint()
nnoremap <silent> [SPC]dp :call vimspector#Pause()
nnoremap <silent> [SPC]dx :call vimspector#RunToCursor()
nnoremap <silent> [SPC]dr :call vimspector#Restart()
nnoremap <silent> [SPC]dc :call vimspector#Continue()
nnoremap <silent> [SPC]ff :UniteWithBufferDir file_rec/neovim
nnoremap <silent> [SPC]Ts :Unite colorscheme
nnoremap <silent> [SPC]pf :Unite file_rec/neovim
nnoremap <silent> [SPC]is :Unite neosnippet
nnoremap <silent> [SPC]rl :Unite resume
nnoremap <silent> [SPC]fr :Unite file_mru
nnoremap <silent> [SPC]iu :Unite unicode
nnoremap <silent> [SPC]hi :UniteWithCursorWord help
nnoremap <silent> [SPC]bb :Unite buffer
nnoremap <silent> [SPC]hm :Unite manpage
nnoremap <silent> [SPC]h[SPC] :Unite help -input=SpaceVim
nnoremap <silent> [SPC]? :Unite menu:CustomKeyMaps -input=[SPC]
nnoremap <silent> [SPC]tmT :if &laststatus == 2 | let &laststatus = 0 | else | let &laststatus = 2 | endif
nnoremap <silent> [SPC]tmp :call SpaceVim#layers#core#statusline#toggle_section("cursorpos")
nnoremap <silent> [SPC]tmt :call SpaceVim#layers#core#statusline#toggle_section("time")
nnoremap <silent> [SPC]tmi :call SpaceVim#layers#core#statusline#toggle_section("input method")
nnoremap <silent> [SPC]tmd :call SpaceVim#layers#core#statusline#toggle_section("date")
nnoremap <silent> [SPC]tmb :call SpaceVim#layers#core#statusline#toggle_section("battery status")
nnoremap <silent> [SPC]tmM :call SpaceVim#layers#core#statusline#toggle_section("major mode")
nnoremap <silent> [SPC]tmm :call SpaceVim#layers#core#statusline#toggle_section("minor mode lighters")
nmap <silent> [SPC]; <Plug>CommentOperator
xmap <silent> [SPC]cP <Plug>CommentParagraphs
nmap <silent> [SPC]cP <Plug>CommentParagraphs
xmap <silent> [SPC]cp <Plug>CommentParagraphsInvert
nmap <silent> [SPC]cp <Plug>CommentParagraphsInvert
xmap <silent> [SPC]cT <Plug>CommentToLine
nmap <silent> [SPC]cT <Plug>CommentToLine
xmap <silent> [SPC]ct <Plug>CommentToLineInvert
nmap <silent> [SPC]ct <Plug>CommentToLineInvert
xmap <silent> [SPC]c$ <Plug>NERDCommenterToEOL
nmap <silent> [SPC]c$ <Plug>NERDCommenterToEOL
xmap <silent> [SPC]cY <Plug>NERDCommenterYank
nmap <silent> [SPC]cY <Plug>NERDCommenterYank
xmap <silent> [SPC]cy <Plug>CommenterInvertYank
nmap <silent> [SPC]cy <Plug>CommenterInvertYank
xmap <silent> [SPC]cs <Plug>NERDCommenterSexy
nmap <silent> [SPC]cs <Plug>NERDCommenterSexy
xmap <silent> [SPC]cv <Plug>NERDCommenterInvertgv
nmap <silent> [SPC]cv <Plug>NERDCommenterInvertgv
xmap <silent> [SPC]cu <Plug>NERDCommenterUncomment
nmap <silent> [SPC]cu <Plug>NERDCommenterUncomment
xmap <silent> [SPC]cL <Plug>NERDCommenterComment
nmap <silent> [SPC]cL <Plug>NERDCommenterComment
xmap <silent> [SPC]cl <Plug>NERDCommenterInvert
nmap <silent> [SPC]cl <Plug>NERDCommenterInvert
xmap <silent> [SPC]ca <Plug>NERDCommenterAltDelims
nmap <silent> [SPC]ca <Plug>NERDCommenterAltDelims
nnoremap <silent> [SPC]qr :
nnoremap <silent> [SPC]qR :
nnoremap <silent> [SPC]qQ :qa!
nnoremap <silent> [SPC]qq :qa
nnoremap <silent> [SPC]p/ :Grepper
nnoremap <silent> [SPC]pp :call SpaceVim#plugins#projectmanager#list()
nnoremap <silent> [SPC]pk :call SpaceVim#plugins#projectmanager#kill_project()
nnoremap <silent> [SPC]ptr :call SpaceVim#plugins#runner#run_task(SpaceVim#plugins#tasks#get())
nnoremap <silent> [SPC]ptc :call SpaceVim#plugins#runner#clear_tasks()
nnoremap <silent> [SPC]ptl :call SpaceVim#plugins#tasks#list()
nnoremap <silent> [SPC]pte :call SpaceVim#plugins#tasks#edit()
nnoremap <silent> [SPC]fvd :SPConfig
nnoremap <silent> [SPC]fvv :let @+=g:spacevim_version | echo g:spacevim_version
xmap <silent> [SPC]fY <Plug>YankGitRemoteURL
nmap <silent> [SPC]fY <Plug>YankGitRemoteURL
nnoremap <silent> [SPC]fy :call SpaceVim#util#CopyToClipboard()
nnoremap <silent> [SPC]bt :exe "Defx -no-toggle " . fnameescape(expand("%:p:h"))
nnoremap <silent> [SPC]fo :Defx  -no-toggle -search=`expand('%:p')` `stridx(expand('%:p'), getcwd()) < 0? expand('%:p:h'): getcwd()`
nnoremap <silent> [SPC]fT :Defx -no-toggle
nnoremap <silent> [SPC]ft :Defx
nnoremap <silent> [SPC]f/ :call SpaceVim#plugins#find#open()
nnoremap <silent> [SPC]fCu :update | e ++ff=dos | setlocal ff=unix | w
nnoremap <silent> [SPC]fCd :update | e ++ff=dos | w
nnoremap <silent> [SPC]fb :BookmarkShowAll
nnoremap <silent> [SPC]bNn :enew
nnoremap <silent> [SPC]bNl :rightbelow vertical new
nnoremap <silent> [SPC]bNk :new
nnoremap <silent> [SPC]bNj :rightbelow new
nnoremap <silent> [SPC]bNh :topleft vertical new
nnoremap <silent> [SPC]bw :setl readonly!
nnoremap <silent> [SPC]bY :normal! ggVG"+y``
nnoremap <silent> [SPC]bP :normal! ggdG"+P
nnoremap <silent> [SPC]bh :Startify
nnoremap <silent> [SPC]bc :call SpaceVim#mapping#clear_saved_buffers()
nnoremap <silent> [SPC]b<C-S-D> :call SpaceVim#mapping#kill_buffer_expr()
nnoremap <silent> [SPC]b<C-S-D> :call SpaceVim#mapping#kill_buffer_expr()
nnoremap <silent> [SPC]b<C-D> :call SpaceVim#mapping#clear_buffers()
nnoremap <silent> [SPC]b :call SpaceVim#mapping#clear_buffers()
nnoremap <silent> [SPC]bD :call SpaceVim#mapping#kill_visible_buffer_choosewin()
nnoremap <silent> [SPC]bd :call SpaceVim#mapping#close_current_buffer()
nnoremap <silent> [SPC]hG :call SpaceVim#plugins#helpgrep#help(expand("<cword>"))
nnoremap <silent> [SPC]hg :call SpaceVim#plugins#helpgrep#help()
nnoremap <silent> [SPC]	 :try | b# | catch | endtry
nnoremap <silent> [SPC]jn i
nnoremap <silent> [SPC]jf 	
nnoremap <silent> [SPC]jb 
nnoremap <silent> [SPC]j$ m`g_
nnoremap <silent> [SPC]j0 m`^
nnoremap <silent> [SPC]hk :LeaderGuide "[KEYs]"
nnoremap <silent> [SPC]hL :SPRuntimeLog
nnoremap <silent> [SPC]hl :SPLayer -l
nnoremap <silent> [SPC]hI :call SpaceVim#issue#report()
nnoremap <silent> [SPC]fS :wall
nnoremap <silent> [p P
nnoremap <silent> [t :tabprevious
nnoremap <silent> [l :lprevious
nnoremap <silent> [b :bN | stopinsert
nnoremap <silent> [e :execute 'move -1-'. v:count1
nnoremap <silent> [  :put! =repeat(nr2char(10), v:count1)
nnoremap <silent> [SPC]tW :call SpaceVim#layers#core#statusline#toggle_mode("wrapline")
nnoremap <silent> [SPC]tl :setlocal list!
nnoremap <silent> [SPC]tP :DelimitMateSwitch
nnoremap <silent> [SPC]tp :call SpaceVim#layers#core#statusline#toggle_mode("paste-mode")
nnoremap <silent> [SPC]tS :call SpaceVim#layers#core#statusline#toggle_mode("spell-checking")
nnoremap <silent> [SPC]tw :call SpaceVim#layers#core#statusline#toggle_mode("whitespace")
nnoremap <silent> [SPC]thc :set cursorcolumn!
nnoremap <silent> [SPC]tf :call SpaceVim#layers#core#statusline#toggle_mode("fill-column-indicator")
nnoremap <silent> [SPC]tt :call SpaceVim#plugins#tabmanager#open()
nnoremap <silent> [SPC]t8 :call SpaceVim#layers#core#statusline#toggle_mode("hi-characters-for-long-lines")
xmap <silent> [SPC]jw :HopWord
nmap <silent> [SPC]jw :HopWord
xmap <silent> [SPC]jl :HopLine
nmap <silent> [SPC]jl :HopLine
xmap <silent> [SPC]jJ :HopChar2
nmap <silent> [SPC]jJ :HopChar2
xmap <silent> [SPC]jj :HopChar1
nmap <silent> [SPC]jj :HopChar1
nnoremap <silent> [SPC]jk j==
nnoremap <silent> [SPC]jm :SplitjoinSplit
nnoremap <silent> [SPC]jo :SplitjoinJoin
xmap <silent> [SPC]xgp <Plug>(grammarous-move-to-previous-error)
nmap <silent> [SPC]xgp <Plug>(grammarous-move-to-previous-error)
xmap <silent> [SPC]xgn <Plug>(grammarous-move-to-next-error)
nmap <silent> [SPC]xgn <Plug>(grammarous-move-to-next-error)
xmap <silent> [SPC]xlU <Plug>UniquifyCaseSenstiveLines
nmap <silent> [SPC]xlU <Plug>UniquifyCaseSenstiveLines
xmap <silent> [SPC]xlu <Plug>UniquifyIgnoreCaseLines
nmap <silent> [SPC]xlu <Plug>UniquifyIgnoreCaseLines
nnoremap <silent> [SPC]xlS :sort
nnoremap <silent> [SPC]xls :sort i
xmap <silent> [SPC]xlr <Plug>ReverseLines
nmap <silent> [SPC]xlr <Plug>ReverseLines
xmap <silent> [SPC]xld <Plug>DuplicateLines
nmap <silent> [SPC]xld <Plug>DuplicateLines
vnoremap <silent> [SPC]xwc :normal! :'<,'>s/\w\+//gn
xmap <silent> [SPC]x~ <Plug>ToggleCase
nmap <silent> [SPC]x~ <Plug>ToggleCase
xmap <silent> [SPC]xU <Plug>Uppercase
nmap <silent> [SPC]xU <Plug>Uppercase
xmap <silent> [SPC]xu <Plug>Lowercase
nmap <silent> [SPC]xu <Plug>Lowercase
nnoremap <silent> [SPC]xdw :StripWhitespace
xmap <silent> [SPC]xa[SPC] :Tabularize /\s\ze\S/l0
nmap <silent> [SPC]xa[SPC] :Tabularize /\s\ze\S/l0
xnoremap <silent> [SPC]xa| :Tabularize /[|｜]
nnoremap <silent> [SPC]xa| :Tabularize /[|｜]
xnoremap <silent> [SPC]xa¦ :Tabularize /¦
nnoremap <silent> [SPC]xa¦ :Tabularize /¦
xnoremap <silent> [SPC]xao :Tabularize /&&\|||\|\.\.\|\*\*\|<<\|>>\|\/\/\|[-+*/.%^><&|?]/l1r1
nnoremap <silent> [SPC]xao :Tabularize /&&\|||\|\.\.\|\*\*\|<<\|>>\|\/\/\|[-+*/.%^><&|?]/l1r1
xnoremap <silent> [SPC]xa= :Tabularize /===\|<=>\|\(&&\|||\|<<\|>>\|\/\/\)=\|=\~[#?]\?\|=>\|[:+/*!%^=><&|.?-]\?=[#?]\?/l1r1
nnoremap <silent> [SPC]xa= :Tabularize /===\|<=>\|\(&&\|||\|<<\|>>\|\/\/\)=\|=\~[#?]\?\|=>\|[:+/*!%^=><&|.?-]\?=[#?]\?/l1r1
xnoremap <silent> [SPC]xa; :Tabularize /;
nnoremap <silent> [SPC]xa; :Tabularize /;
xnoremap <silent> [SPC]xa: :Tabularize /:
nnoremap <silent> [SPC]xa: :Tabularize /:
xnoremap <silent> [SPC]xa. :Tabularize /\.
nnoremap <silent> [SPC]xa. :Tabularize /\.
xnoremap <silent> [SPC]xa, :Tabularize /,
nnoremap <silent> [SPC]xa, :Tabularize /,
xnoremap <silent> [SPC]xa} :Tabularize /}
nnoremap <silent> [SPC]xa} :Tabularize /}
xnoremap <silent> [SPC]xa{ :Tabularize /{
nnoremap <silent> [SPC]xa{ :Tabularize /{
xnoremap <silent> [SPC]xa] :Tabularize /]
nnoremap <silent> [SPC]xa] :Tabularize /]
xnoremap <silent> [SPC]xa[ :Tabularize /[
nnoremap <silent> [SPC]xa[ :Tabularize /[
xnoremap <silent> [SPC]xa) :Tabularize /)
nnoremap <silent> [SPC]xa) :Tabularize /)
xnoremap <silent> [SPC]xa( :Tabularize /(
nnoremap <silent> [SPC]xa( :Tabularize /(
xnoremap <silent> [SPC]xa& :Tabularize /&
nnoremap <silent> [SPC]xa& :Tabularize /&
xnoremap <silent> [SPC]xa% :Tabularize /%
nnoremap <silent> [SPC]xa% :Tabularize /%
xnoremap <silent> [SPC]xa# :Tabularize /#
nnoremap <silent> [SPC]xa# :Tabularize /#
xmap <silent> [SPC]xc <Plug>CountSelectionRegion
nmap <silent> [SPC]xc <Plug>CountSelectionRegion
nnoremap <silent> [SPC]bf :Neoformat
nnoremap <silent> [SPC]xss :NeoSnippetEdit
nnoremap <silent> [SPC]hdk :call SpaceVim#plugins#help#describe_key()
nnoremap <silent> [SPC]sh :call SpaceVim#plugins#highlight#start(0)
nnoremap <silent> [SPC]sH :call SpaceVim#plugins#highlight#start(1)
nnoremap <silent> [SPC]sE :call SpaceVim#plugins#iedit#start({"selectall" : 0})
xmap <silent> [SPC]se <Plug>SpaceVim-plugin-iedit
nmap <silent> [SPC]se <Plug>SpaceVim-plugin-iedit
nnoremap <silent> [SPC]sc :call SpaceVim#plugins#searcher#clear()
nnoremap <silent> [SPC]s/ :call SpaceVim#plugins#flygrep#open({})
nnoremap <silent> [SPC]stJ :call SpaceVim#plugins#searcher#find(expand("<cword>"), "pt")
nnoremap <silent> [SPC]stj :call SpaceVim#plugins#searcher#find("", "pt")
nnoremap <silent> [SPC]stF :call SpaceVim#mapping#search#grep("t", "F")
nnoremap <silent> [SPC]stf :call SpaceVim#mapping#search#grep("t", "f")
nnoremap <silent> [SPC]stP :call SpaceVim#mapping#search#grep("t", "P")
nnoremap <silent> [SPC]stp :call SpaceVim#mapping#search#grep("t", "p")
nnoremap <silent> [SPC]stD :call SpaceVim#mapping#search#grep("t", "D")
nnoremap <silent> [SPC]std :call SpaceVim#mapping#search#grep("t", "d")
nnoremap <silent> [SPC]stB :call SpaceVim#mapping#search#grep("t", "B")
nnoremap <silent> [SPC]stb :call SpaceVim#mapping#search#grep("t", "b")
nnoremap <silent> [SPC]siJ :call SpaceVim#plugins#searcher#find(expand("<cword>"), "findstr")
nnoremap <silent> [SPC]sij :call SpaceVim#plugins#searcher#find("", "findstr")
nnoremap <silent> [SPC]siF :call SpaceVim#mapping#search#grep("i", "F")
nnoremap <silent> [SPC]sif :call SpaceVim#mapping#search#grep("i", "f")
nnoremap <silent> [SPC]siP :call SpaceVim#mapping#search#grep("i", "P")
nnoremap <silent> [SPC]sip :call SpaceVim#mapping#search#grep("i", "p")
nnoremap <silent> [SPC]siD :call SpaceVim#mapping#search#grep("i", "D")
nnoremap <silent> [SPC]sid :call SpaceVim#mapping#search#grep("i", "d")
nnoremap <silent> [SPC]siB :call SpaceVim#mapping#search#grep("i", "B")
nnoremap <silent> [SPC]sib :call SpaceVim#mapping#search#grep("i", "b")
nnoremap <silent> [SPC]srJ :call SpaceVim#plugins#searcher#find(expand("<cword>"), "rg")
nnoremap <silent> [SPC]srj :call SpaceVim#plugins#searcher#find("", "rg")
nnoremap <silent> [SPC]srF :call SpaceVim#mapping#search#grep("r", "F")
nnoremap <silent> [SPC]srf :call SpaceVim#mapping#search#grep("r", "f")
nnoremap <silent> [SPC]srP :call SpaceVim#mapping#search#grep("r", "P")
nnoremap <silent> [SPC]srp :call SpaceVim#mapping#search#grep("r", "p")
nnoremap <silent> [SPC]srD :call SpaceVim#mapping#search#grep("r", "D")
nnoremap <silent> [SPC]srd :call SpaceVim#mapping#search#grep("r", "d")
nnoremap <silent> [SPC]srB :call SpaceVim#mapping#search#grep("r", "B")
nnoremap <silent> [SPC]srb :call SpaceVim#mapping#search#grep("r", "b")
nnoremap <silent> [SPC]skJ :call SpaceVim#plugins#searcher#find(expand("<cword>"), "ack")
nnoremap <silent> [SPC]skj :call SpaceVim#plugins#searcher#find("", "ack")
nnoremap <silent> [SPC]skF :call SpaceVim#mapping#search#grep("k", "F")
nnoremap <silent> [SPC]skf :call SpaceVim#mapping#search#grep("k", "f")
nnoremap <silent> [SPC]skP :call SpaceVim#mapping#search#grep("k", "P")
nnoremap <silent> [SPC]skp :call SpaceVim#mapping#search#grep("k", "p")
nnoremap <silent> [SPC]skD :call SpaceVim#mapping#search#grep("k", "D")
nnoremap <silent> [SPC]skd :call SpaceVim#mapping#search#grep("k", "d")
nnoremap <silent> [SPC]skB :call SpaceVim#mapping#search#grep("k", "B")
nnoremap <silent> [SPC]skb :call SpaceVim#mapping#search#grep("k", "b")
nnoremap <silent> [SPC]sGF :call SpaceVim#mapping#search#grep("G", "F")
nnoremap <silent> [SPC]sGf :call SpaceVim#mapping#search#grep("G", "f")
nnoremap <silent> [SPC]sGP :call SpaceVim#mapping#search#grep("G", "P")
nnoremap <silent> [SPC]sGp :call SpaceVim#mapping#search#grep("G", "p")
nnoremap <silent> [SPC]sGD :call SpaceVim#mapping#search#grep("G", "D")
nnoremap <silent> [SPC]sGd :call SpaceVim#mapping#search#grep("G", "d")
nnoremap <silent> [SPC]sGB :call SpaceVim#mapping#search#grep("G", "B")
nnoremap <silent> [SPC]sGb :call SpaceVim#mapping#search#grep("G", "b")
nnoremap <silent> [SPC]sgJ :call SpaceVim#plugins#searcher#find(expand("<cword>"), "grep")
nnoremap <silent> [SPC]sgj :call SpaceVim#plugins#searcher#find("", "grep")
nnoremap <silent> [SPC]sgF :call SpaceVim#mapping#search#grep("g", "F")
nnoremap <silent> [SPC]sgf :call SpaceVim#mapping#search#grep("g", "f")
nnoremap <silent> [SPC]sgP :call SpaceVim#mapping#search#grep("g", "P")
nnoremap <silent> [SPC]sgp :call SpaceVim#mapping#search#grep("g", "p")
nnoremap <silent> [SPC]sgD :call SpaceVim#mapping#search#grep("g", "D")
nnoremap <silent> [SPC]sgd :call SpaceVim#mapping#search#grep("g", "d")
nnoremap <silent> [SPC]sgB :call SpaceVim#mapping#search#grep("g", "B")
nnoremap <silent> [SPC]sgb :call SpaceVim#mapping#search#grep("g", "b")
nnoremap <silent> [SPC]saJ :call SpaceVim#plugins#searcher#find(expand("<cword>"), "ag")
nnoremap <silent> [SPC]saj :call SpaceVim#plugins#searcher#find("", "ag")
nnoremap <silent> [SPC]saF :call SpaceVim#mapping#search#grep("a", "F")
nnoremap <silent> [SPC]saf :call SpaceVim#mapping#search#grep("a", "f")
nnoremap <silent> [SPC]saP :call SpaceVim#mapping#search#grep("a", "P")
nnoremap <silent> [SPC]sap :call SpaceVim#mapping#search#grep("a", "p")
nnoremap <silent> [SPC]saD :call SpaceVim#mapping#search#grep("a", "D")
nnoremap <silent> [SPC]sad :call SpaceVim#mapping#search#grep("a", "d")
nnoremap <silent> [SPC]saB :call SpaceVim#mapping#search#grep("a", "B")
nnoremap <silent> [SPC]sab :call SpaceVim#mapping#search#grep("a", "b")
nnoremap <silent> [SPC]sl :call SpaceVim#plugins#searcher#list()
nnoremap <silent> [SPC]sJ :call SpaceVim#plugins#searcher#find(expand("<cword>"),SpaceVim#mapping#search#default_tool()[0])
nnoremap <silent> [SPC]sj :lua require('spacevim.plugin.searcher').find('', require('spacevim.plugin.search').default_tool())
nnoremap <silent> [SPC]sP :call SpaceVim#plugins#flygrep#open({'input' : expand("<cword>"), 'dir' : get(b:, "rootDir", getcwd())})
nnoremap <silent> [SPC]sp :call SpaceVim#plugins#flygrep#open({'input' : input("grep pattern:"), 'dir' : get(b:, "rootDir", getcwd())})
nnoremap <silent> [SPC]sF :call SpaceVim#plugins#flygrep#open({"input" : expand("<cword>"), "dir": input("arbitrary dir:", "", "dir")})
nnoremap <silent> [SPC]sf :call SpaceVim#plugins#flygrep#open({"input" : input("grep pattern:"), "dir": input("arbitrary dir:", "", "dir")})
nnoremap <silent> [SPC]sD :call SpaceVim#plugins#flygrep#open({"input" : expand("<cword>"), "dir": fnamemodify(expand("%"), ":p:h")})
nnoremap <silent> [SPC]sd :call SpaceVim#plugins#flygrep#open({"input" : input("grep pattern:"), "dir": fnamemodify(expand("%"), ":p:h")})
nnoremap <silent> [SPC]sB :call SpaceVim#plugins#flygrep#open({"input" : expand("<cword>"), "files": "@buffers"})
nnoremap <silent> [SPC]sb :call SpaceVim#plugins#flygrep#open({"input" : input("grep pattern:"), "files": "@buffers"})
nnoremap <silent> [SPC]sS :call SpaceVim#plugins#flygrep#open({"input" : expand("<cword>"), "files": bufname("%")})
nnoremap <silent> [SPC]ss :call SpaceVim#plugins#flygrep#open({"input" : input("grep pattern:"), "files": bufname("%")})
nnoremap <silent> [SPC]tn :setlocal nonumber! norelativenumber!
nnoremap <silent> [SPC]wU :call SpaceVim#plugins#windowsmanager#RedoQuitWin()
nnoremap <silent> [SPC]wu :call SpaceVim#plugins#windowsmanager#UndoQuitWin()
nnoremap <silent> [SPC]wW :ChooseWin
nnoremap <silent> [SPC]ww :wincmd w
nnoremap <silent> [SPC]w= :wincmd =
nnoremap <silent> [SPC]wV :bel vs
nnoremap <silent> [SPC]w3 :silent only | vs | vs | wincmd H
nnoremap <silent> [SPC]w2 :silent only | vs | wincmd w
nnoremap <silent> [SPC]wS :bel split
nnoremap <silent> [SPC]ws :bel split | wincmd w
nnoremap <silent> [SPC]w- :bel split | wincmd w
nnoremap <silent> [SPC]wv :belowright vsplit | wincmd w
nnoremap <silent> [SPC]w/ :belowright vsplit | wincmd w
nnoremap <silent> [SPC]wo :tabnext
nnoremap <silent> [SPC]wM :execute eval("winnr('$')<=2 ? 'wincmd x' : 'ChooseWinSwap'")
nnoremap <silent> [SPC]wL :wincmd L
nnoremap <silent> [SPC]wK :wincmd K
nnoremap <silent> [SPC]wJ :wincmd J
nnoremap <silent> [SPC]wH :wincmd H
nnoremap <silent> [SPC]wl :wincmd l
nnoremap <silent> [SPC]wk :wincmd k
nnoremap <silent> [SPC]wx :wincmd x
nnoremap <silent> [SPC]wj :wincmd j
nnoremap <silent> [SPC]wh :wincmd h
nnoremap <silent> [SPC]wF :tabnew
nnoremap <silent> [SPC]wD :ChooseWin | close | wincmd w
nnoremap <silent> [SPC]wf :setlocal scrollbind!
nnoremap <silent> [SPC]wd :close
nnoremap <silent> [SPC]w	 :wincmd w
nnoremap <silent> [SPC]9 :call SpaceVim#layers#core#statusline#jump(9)
nnoremap <silent> [SPC]8 :call SpaceVim#layers#core#statusline#jump(8)
nnoremap <silent> [SPC]7 :call SpaceVim#layers#core#statusline#jump(7)
nnoremap <silent> [SPC]6 :call SpaceVim#layers#core#statusline#jump(6)
nnoremap <silent> [SPC]5 :call SpaceVim#layers#core#statusline#jump(5)
nnoremap <silent> [SPC]4 :call SpaceVim#layers#core#statusline#jump(4)
nnoremap <silent> [SPC]3 :call SpaceVim#layers#core#statusline#jump(3)
nnoremap <silent> [SPC]2 :call SpaceVim#layers#core#statusline#jump(2)
nnoremap <silent> [SPC]1 :call SpaceVim#layers#core#statusline#jump(1)
vnoremap <nowait> <silent> [SPC] :LeaderGuideVisual ' '
nnoremap <nowait> <silent> [SPC] :LeaderGuide ' '
nnoremap <nowait> <silent> [Z] :LeaderGuide "z"
nnoremap <nowait> <silent> [G] :LeaderGuide "g"
nnoremap <silent> [Window]c :call SpaceVim#mapping#clear_buffers()
nnoremap <silent> [Window]q :call SpaceVim#mapping#close_current_buffer()
nnoremap <silent> [Window]Q :close
nnoremap <silent> [Window]\ :b#
nnoremap <silent> [Window]x :call SpaceVim#mapping#BufferEmpty()
nnoremap <silent> [Window]o :only | doautocmd WinEnter
nnoremap <silent> [Window]t :tabnew
nnoremap <silent> [Window]G :vsplit +bp
nnoremap <silent> [Window]g :vsplit
nnoremap <silent> [Window]V :split +bp
nnoremap <silent> [Window]v :split
nnoremap <nowait> <silent> [Window] :LeaderGuide "s"
nmap \qf <Plug>(coc-fix-current)
nmap \as <Plug>(coc-codeaction-source)
nmap \ac <Plug>(coc-codeaction-cursor)
nmap \a <Plug>(coc-codeaction-selected)
xmap \a <Plug>(coc-codeaction-selected)
nmap \rn <Plug>(coc-rename)
xmap \T <Plug>(table-mode-tableize-delimiter)
xmap \tt <Plug>(table-mode-tableize)
nmap \tt <Plug>(table-mode-tableize)
nnoremap <silent> \tm :call tablemode#Toggle()
nnoremap <silent> \fp :Unite menu:AddedPlugins
nnoremap <silent> \f  :Unite menu:CustomKeyMaps
nnoremap <silent> \fo :Unite outline
nnoremap <silent> \fq :Unite quickfix
nnoremap <silent> \fm :Unite output:message
nnoremap <silent> \fl :Unite locationlist
nnoremap <silent> \fj :Unite jump
nnoremap <silent> \fh :Unite history/yank
nnoremap <silent> \fe :Unite -buffer-name=register register
nnoremap <silent> \fr :Unite -buffer-name=resume resume
nmap <silent> \) :call SpaceVim#layers#core#tabline#jump(20)
nmap <silent> \( :call SpaceVim#layers#core#tabline#jump(19)
nmap <silent> \* :call SpaceVim#layers#core#tabline#jump(18)
nmap <silent> \& :call SpaceVim#layers#core#tabline#jump(17)
nmap <silent> \^ :call SpaceVim#layers#core#tabline#jump(16)
nmap <silent> \% :call SpaceVim#layers#core#tabline#jump(15)
nmap <silent> \$ :call SpaceVim#layers#core#tabline#jump(14)
nmap <silent> \# :call SpaceVim#layers#core#tabline#jump(13)
nmap <silent> \@ :call SpaceVim#layers#core#tabline#jump(12)
nmap <silent> \! :call SpaceVim#layers#core#tabline#jump(11)
nmap <silent> \0 :call SpaceVim#layers#core#tabline#jump(10)
nmap <silent> \9 :call SpaceVim#layers#core#tabline#jump(9)
nmap <silent> \8 :call SpaceVim#layers#core#tabline#jump(8)
nmap <silent> \7 :call SpaceVim#layers#core#tabline#jump(7)
nmap <silent> \6 :call SpaceVim#layers#core#tabline#jump(6)
nmap <silent> \5 :call SpaceVim#layers#core#tabline#jump(5)
nmap <silent> \4 :call SpaceVim#layers#core#tabline#jump(4)
nmap <silent> \3 :call SpaceVim#layers#core#tabline#jump(3)
nmap <silent> \2 :call SpaceVim#layers#core#tabline#jump(2)
nmap <silent> \1 :call SpaceVim#layers#core#tabline#jump(1)
vnoremap <silent> \ :LeaderGuideVisual get(g:, 'mapleader', '\')
nnoremap <nowait> <silent> \ :LeaderGuide get(g:, 'mapleader', '\')
nnoremap <silent> \qc :call setqflist([])
nnoremap <silent> \qr q
nnoremap \ql :copen
nnoremap \qp :cprev
nnoremap \qn :cnext
xnoremap <silent> \Y :call SpaceVim#plugins#pastebin#paste()
xnoremap <expr> \P clipboard#paste('P')
xnoremap <expr> \p clipboard#paste('p')
nnoremap <expr> \P clipboard#paste('P')
nnoremap <expr> \p clipboard#paste('p')
xnoremap <silent> \y :call clipboard#yank()
nmap <silent> ]g <Plug>(coc-diagnostic-next)
omap <silent> ]% <Plug>(matchup-]%)
xmap <silent> ]% <Plug>(matchup-]%)
nmap <silent> ]% <Plug>(matchup-]%)
nnoremap <silent> ]p p
nnoremap <silent> ]t :tabnext
nnoremap <silent> ]l :lnext
nnoremap <silent> ]b :bn | stopinsert
nnoremap <silent> ]e :execute 'move +'. v:count1
nnoremap <silent> ]  :put =repeat(nr2char(10), v:count1)
vnoremap <silent> ]<Home> dggP``
vnoremap <silent> ]<End> dGp``
nnoremap <silent> ]<Home> ddggP``
nnoremap <silent> ]<End> ddGp``
omap ae <Plug>(textobj-entire-a)
xmap ae <Plug>(textobj-entire-a)
omap <silent> a% <Plug>(matchup-a%)
xmap <silent> a% <Plug>(matchup-a%)
omap ai <Plug>(textobj-indent-a)
xmap ai <Plug>(textobj-indent-a)
omap aI <Plug>(textobj-indent-same-a)
xmap aI <Plug>(textobj-indent-same-a)
omap al <Plug>(textobj-line-a)
xmap al <Plug>(textobj-line-a)
nmap cS <Plug>CSurround
nmap cs <Plug>Csurround
nmap ds <Plug>Dsurround
omap f <Plug>(clever-f-f)
xmap f <Plug>(clever-f-f)
nmap f <Plug>(clever-f-f)
nmap <silent> gy <Plug>(coc-type-definition)
xmap gS <Plug>VgSurround
omap <silent> g% <Plug>(matchup-g%)
xmap <silent> g% <Plug>(matchup-g%)
nmap <silent> g% <Plug>(matchup-g%)
vmap gx <Plug>(openbrowser-smart-search)
nmap gx <Plug>(openbrowser-smart-search)
vmap gs <Plug>(openbrowser-search)
vmap go <Plug>(openbrowser-open)
nnoremap <silent> <expr> gp '`['.strpart(getregtype(), 0, 1).'`]'
nnoremap <silent> g= :call SpaceVim#mapping#format()
nmap <silent> gd <Plug>(coc-definition)
nnoremap <silent> gD :call SpaceVim#mapping#g_capital_d()
nnoremap g<C-]> g
nnoremap g g
nnoremap gv gv
nmap gs <Plug>(openbrowser-search)
nmap go <Plug>(openbrowser-open)
nnoremap gm gm
nnoremap gg gg
nnoremap ga ga
nnoremap g~ g~
nnoremap g_ g_
nnoremap g^ g^
nnoremap g] g]
nnoremap gt gt
nnoremap gT gT
nnoremap gR gR
nnoremap gq gq
nnoremap gQ gQ
nnoremap gn gn
nnoremap gN gN
nnoremap gJ gJ
nmap <silent> gi <Plug>(coc-implementation)
nnoremap gI gI
nnoremap gh gh
nnoremap gH gH
nnoremap gU gU
nnoremap gE gE
nnoremap gu gu
nnoremap gk gk
nnoremap gj gj
nnoremap gF gF
nnoremap gf gf
nnoremap g< g<
nnoremap ge ge
nnoremap g<Home> g<Home>
nnoremap <silent> g0 :tabfirst
nnoremap g<End> g<End>
nnoremap <silent> g$ :tablast
nnoremap g@ g@
nnoremap g; g;
nnoremap g, g,
nnoremap g- g-
nnoremap g+ g+
nnoremap g` g`
nnoremap g' g'
nnoremap g& g&
nnoremap g<C-G> g
nnoremap g g
nmap g [G]
nmap <silent> gr <Plug>(coc-references)
omap ie <Plug>(textobj-entire-i)
xmap ie <Plug>(textobj-entire-i)
omap <silent> i% <Plug>(matchup-i%)
xmap <silent> i% <Plug>(matchup-i%)
omap ii <Plug>(textobj-indent-i)
xmap ii <Plug>(textobj-indent-i)
omap iI <Plug>(textobj-indent-same-i)
xmap iI <Plug>(textobj-indent-same-i)
omap il <Plug>(textobj-line-i)
xmap il <Plug>(textobj-line-i)
nnoremap <silent> q :call SpaceVim#mapping#SmartClose()
nmap s [Window]
omap t <Plug>(clever-f-t)
xmap t <Plug>(clever-f-t)
nmap t <Plug>(clever-f-t)
xmap v <Plug>(expand_region_expand)
nmap ySS <Plug>YSsurround
nmap ySs <Plug>YSsurround
nmap yss <Plug>Yssurround
nmap yS <Plug>YSurround
nmap ys <Plug>Ysurround
omap <silent> z% <Plug>(matchup-z%)
xmap <silent> z% <Plug>(matchup-z%)
nmap <silent> z% <Plug>(matchup-z%)
nnoremap zx zx
nnoremap zw zw
nnoremap zv zv
nnoremap zt zt
nnoremap zs zs
nnoremap zr zr
nnoremap zo zo
nnoremap zn zn
nnoremap zm zm
nnoremap z<Right> zl
nnoremap zl zl
nnoremap zK zkzx
nnoremap zk zk
nnoremap zJ zjzx
nnoremap zj zj
nnoremap zi zi
nnoremap z<Left> zh
nnoremap zh zh
nnoremap zg zg
nnoremap zf zf
nnoremap ze ze
nnoremap zd zd
nnoremap zc zc
nnoremap zb zb
nnoremap za za
nnoremap zX zX
nnoremap zW zW
nnoremap zR zR
nnoremap zO zO
nnoremap zN zN
nnoremap zM zM
nnoremap zL zL
nnoremap zH zH
nnoremap zG zG
nnoremap zF zF
nnoremap zE zE
nnoremap zD zD
nnoremap zC zC
nnoremap zA zA
nnoremap z= z=
nnoremap z. z.
nnoremap z^ z^
nnoremap z- z-
nnoremap z+ z+
nnoremap z z
nmap z [Z]
nnoremap zz zz
nnoremap <silent> <Plug>(choosewin) :call choosewin#start(range(1, winnr('$')))
xnoremap <silent> <Plug>(neosnippet_register_oneshot_snippet) :call neosnippet#mappings#_register_oneshot_snippet()
xnoremap <silent> <Plug>(neosnippet_expand_target) :call neosnippet#mappings#_expand_target()
xnoremap <silent> <Plug>(neosnippet_get_selected_text) :call neosnippet#helpers#get_selected_text(visualmode(), 1)
xmap <Plug>(neosnippet_jump) <Plug>(neosnippet_jump)
xmap <Plug>(neosnippet_expand) <Plug>(neosnippet_expand)
xmap <Plug>(neosnippet_jump_or_expand) <Plug>(neosnippet_jump_or_expand)
xmap <Plug>(neosnippet_expand_or_jump) <Plug>(neosnippet_expand_or_jump)
nmap <Plug>(neosnippet_jump) g<Plug>(neosnippet_jump)
nmap <Plug>(neosnippet_expand) g<Plug>(neosnippet_expand)
nmap <Plug>(neosnippet_jump_or_expand) g<Plug>(neosnippet_jump_or_expand)
nmap <Plug>(neosnippet_expand_or_jump) g<Plug>(neosnippet_expand_or_jump)
snoremap <silent> <expr> <Plug>(neosnippet_jump) neosnippet#mappings#jump_impl()
snoremap <silent> <expr> <Plug>(neosnippet_expand) neosnippet#mappings#expand_impl()
snoremap <silent> <expr> <Plug>(neosnippet_jump_or_expand) neosnippet#mappings#jump_or_expand_impl()
snoremap <silent> <expr> <Plug>(neosnippet_expand_or_jump) neosnippet#mappings#expand_or_jump_impl()
nmap <Plug>(unite_source_outline_loop_cursor_up) <Plug>(unite_skip_cursor_up)
nmap <Plug>(unite_source_outline_loop_cursor_down) <Plug>(unite_skip_cursor_down)
vnoremap <silent> <Plug>(coc-snippets-select) :call coc#rpc#notify('doKeymap', ['coc-snippets-select'])
xnoremap <silent> <Plug>(coc-convert-snippet) :call coc#rpc#notify('doKeymap', ['coc-convert-snippet'])
vnoremap <nowait> <silent> <expr> <C-B> coc#float#has_scroll() ? coc#float#scroll(0) : "\"
vnoremap <nowait> <silent> <expr> <C-F> coc#float#has_scroll() ? coc#float#scroll(1) : "\"
nnoremap <nowait> <silent> <expr> <C-B> coc#float#has_scroll() ? coc#float#scroll(0) : "\"
nnoremap <nowait> <silent> <expr> <C-F> coc#float#has_scroll() ? coc#float#scroll(1) : "\"
xnoremap <silent> <Plug>NERDCommenterUncomment :call NERDComment("x", "Uncomment")
nnoremap <silent> <Plug>NERDCommenterUncomment :call NERDComment("n", "Uncomment")
xnoremap <silent> <Plug>NERDCommenterAlignBoth :call NERDComment("x", "AlignBoth")
nnoremap <silent> <Plug>NERDCommenterAlignBoth :call NERDComment("n", "AlignBoth")
xnoremap <silent> <Plug>NERDCommenterAlignLeft :call NERDComment("x", "AlignLeft")
nnoremap <silent> <Plug>NERDCommenterAlignLeft :call NERDComment("n", "AlignLeft")
nnoremap <silent> <Plug>NERDCommenterAppend :call NERDComment("n", "Append")
xnoremap <silent> <Plug>NERDCommenterYank :call NERDComment("x", "Yank")
nnoremap <silent> <Plug>NERDCommenterYank :call NERDComment("n", "Yank")
xnoremap <silent> <Plug>NERDCommenterSexy :call NERDComment("x", "Sexy")
nnoremap <silent> <Plug>NERDCommenterSexy :call NERDComment("n", "Sexy")
xnoremap <silent> <Plug>NERDCommenterInvert :call NERDComment("x", "Invert")
nnoremap <silent> <Plug>NERDCommenterInvert :call NERDComment("n", "Invert")
nnoremap <silent> <Plug>NERDCommenterToEOL :call NERDComment("n", "ToEOL")
xnoremap <silent> <Plug>NERDCommenterNested :call NERDComment("x", "Nested")
nnoremap <silent> <Plug>NERDCommenterNested :call NERDComment("n", "Nested")
xnoremap <silent> <Plug>NERDCommenterMinimal :call NERDComment("x", "Minimal")
nnoremap <silent> <Plug>NERDCommenterMinimal :call NERDComment("n", "Minimal")
xnoremap <silent> <Plug>NERDCommenterToggle :call NERDComment("x", "Toggle")
nnoremap <silent> <Plug>NERDCommenterToggle :call NERDComment("n", "Toggle")
xnoremap <silent> <Plug>NERDCommenterComment :call NERDComment("x", "Comment")
nnoremap <silent> <Plug>NERDCommenterComment :call NERDComment("n", "Comment")
noremap <silent> <expr> <Plug>(clever-f-repeat-back) clever_f#repeat(1)
noremap <silent> <expr> <Plug>(clever-f-repeat-forward) clever_f#repeat(0)
noremap <silent> <expr> <Plug>(clever-f-reset) clever_f#reset()
noremap <silent> <expr> <Plug>(clever-f-T) clever_f#find_with('T')
noremap <silent> <expr> <Plug>(clever-f-t) clever_f#find_with('t')
noremap <silent> <expr> <Plug>(clever-f-F) clever_f#find_with('F')
noremap <silent> <expr> <Plug>(clever-f-f) clever_f#find_with('f')
nnoremap <silent> <Plug>SurroundRepeat .
nnoremap <silent> <Plug>(table-mode-sort) :call tablemode#spreadsheet#Sort('')
nnoremap <silent> <Plug>(table-mode-eval-formula) :call tablemode#spreadsheet#formula#EvaluateFormulaLine()
nnoremap <silent> <Plug>(table-mode-add-formula) :call tablemode#spreadsheet#formula#Add()
nnoremap <silent> <Plug>(table-mode-insert-column-after) :call tablemode#spreadsheet#InsertColumn(1)
nnoremap <silent> <Plug>(table-mode-insert-column-before) :call tablemode#spreadsheet#InsertColumn(0)
nnoremap <silent> <Plug>(table-mode-delete-column) :call tablemode#spreadsheet#DeleteColumn()
nnoremap <silent> <Plug>(table-mode-delete-row) :call tablemode#spreadsheet#DeleteRow()
xnoremap <silent> <Plug>(table-mode-cell-text-object-i) :call tablemode#spreadsheet#cell#TextObject(1)
xnoremap <silent> <Plug>(table-mode-cell-text-object-a) :call tablemode#spreadsheet#cell#TextObject(0)
onoremap <silent> <Plug>(table-mode-cell-text-object-i) :call tablemode#spreadsheet#cell#TextObject(1)
onoremap <silent> <Plug>(table-mode-cell-text-object-a) :call tablemode#spreadsheet#cell#TextObject(0)
nnoremap <silent> <Plug>(table-mode-motion-right) :call tablemode#spreadsheet#cell#Motion('l')
nnoremap <silent> <Plug>(table-mode-motion-left) :call tablemode#spreadsheet#cell#Motion('h')
nnoremap <silent> <Plug>(table-mode-motion-down) :call tablemode#spreadsheet#cell#Motion('j')
nnoremap <silent> <Plug>(table-mode-motion-up) :call tablemode#spreadsheet#cell#Motion('k')
nnoremap <silent> <Plug>(table-mode-realign) :call tablemode#table#Realign('.')
xnoremap <silent> <Plug>(table-mode-tableize-delimiter) :call tablemode#TableizeByDelimiter()
xnoremap <silent> <Plug>(table-mode-tableize) :Tableize
nnoremap <silent> <Plug>(table-mode-tableize) :Tableize
nnoremap <silent> <Plug>(startify-open-buffers) :call startify#open_buffers()
vnoremap <silent> <F25> :silent doautocmd <nomodeline> FocusGained %gv
vnoremap <silent> <F24> :silent doautocmd <nomodeline> FocusLost %gv
onoremap <silent> <F25> :silent doautocmd <nomodeline> FocusGained %
onoremap <silent> <F24> :silent doautocmd <nomodeline> FocusLost %
nnoremap <silent> <F25> :doautocmd <nomodeline> FocusGained %
nnoremap <silent> <F24> :silent doautocmd <nomodeline> FocusLost %
xnoremap <silent> <Plug>(openbrowser-smart-search) :call openbrowser#_keymap_smart_search('v')
nnoremap <silent> <Plug>(openbrowser-smart-search) :call openbrowser#_keymap_smart_search('n')
xnoremap <silent> <Plug>(openbrowser-search) :call openbrowser#_keymap_search('v')
nnoremap <silent> <Plug>(openbrowser-search) :call openbrowser#_keymap_search('n')
xnoremap <silent> <Plug>(openbrowser-open-incognito) :call openbrowser#_keymap_open('v', 0, ['--incognito'])
nnoremap <silent> <Plug>(openbrowser-open-incognito) :call openbrowser#_keymap_open('n', 0, ['--incognito'])
xnoremap <silent> <Plug>(openbrowser-open) :call openbrowser#_keymap_open('v')
nnoremap <silent> <Plug>(openbrowser-open) :call openbrowser#_keymap_open('n')
xnoremap <silent> <Plug>(expand_region_shrink) :call expand_region#next('v', '-')
xnoremap <silent> <Plug>(expand_region_expand) :call expand_region#next('v', '+')
nnoremap <silent> <Plug>(expand_region_expand) :call expand_region#next('n', '+')
snoremap <silent> <Plug>(complete_parameter#overload_up) :call cmp#overload_next(0)
nnoremap <silent> <Plug>(complete_parameter#overload_up) :call cmp#overload_next(0)
snoremap <silent> <Plug>(complete_parameter#overload_down) :call cmp#overload_next(1)
nnoremap <silent> <Plug>(complete_parameter#overload_down) :call cmp#overload_next(1)
snoremap <silent> <Plug>(complete_parameter#goto_previous_parameter) :call cmp#goto_next_param(0)
nnoremap <silent> <Plug>(complete_parameter#goto_previous_parameter) :call cmp#goto_next_param(0)
snoremap <silent> <Plug>(complete_parameter#goto_next_parameter) :call cmp#goto_next_param(1)
nnoremap <silent> <Plug>(complete_parameter#goto_next_parameter) :call cmp#goto_next_param(1)
nmap <silent> <2-LeftMouse> <Plug>(matchup-double-click)
nnoremap <Plug>(matchup-reload) :MatchupReload
nnoremap <silent> <Plug>(matchup-double-click) :call matchup#text_obj#double_click()
onoremap <silent> <Plug>(matchup-a%) :call matchup#text_obj#delimited(0, 0, 'delim_all')
onoremap <silent> <Plug>(matchup-i%) :call matchup#text_obj#delimited(1, 0, 'delim_all')
xnoremap <silent> <Plug>(matchup-a%) :call matchup#text_obj#delimited(0, 1, 'delim_all')
xnoremap <silent> <Plug>(matchup-i%) :call matchup#text_obj#delimited(1, 1, 'delim_all')
onoremap <silent> <Plug>(matchup-z%) :call matchup#motion#op('z%')
xnoremap <silent> <SNR>130_(matchup-z%) :call matchup#motion#jump_inside(1)
nnoremap <silent> <Plug>(matchup-z%) :call matchup#motion#jump_inside(0)
onoremap <silent> <Plug>(matchup-[%) :call matchup#motion#op('[%')
onoremap <silent> <Plug>(matchup-]%) :call matchup#motion#op(']%')
xnoremap <silent> <SNR>130_(matchup-[%) :call matchup#motion#find_unmatched(1, 0)
xnoremap <silent> <SNR>130_(matchup-]%) :call matchup#motion#find_unmatched(1, 1)
nnoremap <silent> <Plug>(matchup-[%) :call matchup#motion#find_unmatched(0, 0)
nnoremap <silent> <Plug>(matchup-]%) :call matchup#motion#find_unmatched(0, 1)
onoremap <silent> <Plug>(matchup-g%) :call matchup#motion#op('g%')
xnoremap <silent> <SNR>130_(matchup-g%) :call matchup#motion#find_matching_pair(1, 0)
onoremap <silent> <Plug>(matchup-%) :call matchup#motion#op('%')
xnoremap <silent> <SNR>130_(matchup-%) :call matchup#motion#find_matching_pair(1, 1)
nnoremap <silent> <Plug>(matchup-g%) :call matchup#motion#find_matching_pair(0, 0)
nnoremap <silent> <Plug>(matchup-%) :call matchup#motion#find_matching_pair(0, 1)
nnoremap <silent> <expr> <SNR>130_(wise) empty(g:v_motion_force) ? 'v' : g:v_motion_force
nnoremap <silent> <Plug>(matchup-hi-surround) :call matchup#matchparen#highlight_surrounding()
onoremap <silent> <Plug>(coc-classobj-a) :call CocAction('selectSymbolRange', v:false, '', ['Interface', 'Struct', 'Class'])
onoremap <silent> <Plug>(coc-classobj-i) :call CocAction('selectSymbolRange', v:true, '', ['Interface', 'Struct', 'Class'])
vnoremap <silent> <Plug>(coc-classobj-a) :call CocAction('selectSymbolRange', v:false, visualmode(), ['Interface', 'Struct', 'Class'])
vnoremap <silent> <Plug>(coc-classobj-i) :call CocAction('selectSymbolRange', v:true, visualmode(), ['Interface', 'Struct', 'Class'])
onoremap <silent> <Plug>(coc-funcobj-a) :call CocAction('selectSymbolRange', v:false, '', ['Method', 'Function'])
onoremap <silent> <Plug>(coc-funcobj-i) :call CocAction('selectSymbolRange', v:true, '', ['Method', 'Function'])
vnoremap <silent> <Plug>(coc-funcobj-a) :call CocAction('selectSymbolRange', v:false, visualmode(), ['Method', 'Function'])
vnoremap <silent> <Plug>(coc-funcobj-i) :call CocAction('selectSymbolRange', v:true, visualmode(), ['Method', 'Function'])
nnoremap <silent> <Plug>(coc-cursors-position) :call CocAction('cursorsSelect', bufnr('%'), 'position', 'n')
nnoremap <silent> <Plug>(coc-cursors-word) :call CocAction('cursorsSelect', bufnr('%'), 'word', 'n')
vnoremap <silent> <Plug>(coc-cursors-range) :call CocAction('cursorsSelect', bufnr('%'), 'range', visualmode())
nnoremap <silent> <Plug>(coc-refactor) :call       CocActionAsync('refactor')
nnoremap <silent> <Plug>(coc-command-repeat) :call       CocAction('repeatCommand')
nnoremap <silent> <Plug>(coc-float-jump) :call       coc#float#jump()
nnoremap <silent> <Plug>(coc-float-hide) :call       coc#float#close_all()
nnoremap <silent> <Plug>(coc-fix-current) :call       CocActionAsync('doQuickfix')
nnoremap <silent> <Plug>(coc-openlink) :call       CocActionAsync('openLink')
nnoremap <silent> <Plug>(coc-references-used) :call       CocActionAsync('jumpUsed')
nnoremap <silent> <Plug>(coc-references) :call       CocActionAsync('jumpReferences')
nnoremap <silent> <Plug>(coc-type-definition) :call       CocActionAsync('jumpTypeDefinition')
nnoremap <silent> <Plug>(coc-implementation) :call       CocActionAsync('jumpImplementation')
nnoremap <silent> <Plug>(coc-declaration) :call       CocActionAsync('jumpDeclaration')
nnoremap <silent> <Plug>(coc-definition) :call       CocActionAsync('jumpDefinition')
nnoremap <silent> <Plug>(coc-diagnostic-prev-error) :call       CocActionAsync('diagnosticPrevious', 'error')
nnoremap <silent> <Plug>(coc-diagnostic-next-error) :call       CocActionAsync('diagnosticNext',     'error')
nnoremap <silent> <Plug>(coc-diagnostic-prev) :call       CocActionAsync('diagnosticPrevious')
nnoremap <silent> <Plug>(coc-diagnostic-next) :call       CocActionAsync('diagnosticNext')
nnoremap <silent> <Plug>(coc-diagnostic-info) :call       CocActionAsync('diagnosticInfo')
nnoremap <silent> <Plug>(coc-format) :call       CocActionAsync('format')
nnoremap <silent> <Plug>(coc-rename) :call       CocActionAsync('rename')
nnoremap <Plug>(coc-codeaction-source) :call       CocActionAsync('codeAction', '', ['source'], v:true)
nnoremap <Plug>(coc-codeaction-refactor) :call       CocActionAsync('codeAction', 'cursor', ['refactor'], v:true)
nnoremap <Plug>(coc-codeaction-cursor) :call       CocActionAsync('codeAction', 'cursor')
nnoremap <Plug>(coc-codeaction-line) :call       CocActionAsync('codeAction', 'currline')
nnoremap <Plug>(coc-codeaction) :call       CocActionAsync('codeAction', '')
vnoremap <Plug>(coc-codeaction-refactor-selected) :call       CocActionAsync('codeAction', visualmode(), ['refactor'], v:true)
vnoremap <silent> <Plug>(coc-codeaction-selected) :call       CocActionAsync('codeAction', visualmode())
vnoremap <silent> <Plug>(coc-format-selected) :call       CocActionAsync('formatSelected', visualmode())
nnoremap <Plug>(coc-codelens-action) :call       CocActionAsync('codeLensAction')
nnoremap <Plug>(coc-range-select) :call       CocActionAsync('rangeSelect',     '', v:true)
vnoremap <silent> <Plug>(coc-range-select-backward) :call       CocActionAsync('rangeSelect',     visualmode(), v:false)
vnoremap <silent> <Plug>(coc-range-select) :call       CocActionAsync('rangeSelect',     visualmode(), v:true)
nnoremap <silent> <Plug>VimspectorDisassemble :call vimspector#ShowDisassembly()
nnoremap <silent> <Plug>VimspectorBreakpoints :call vimspector#ListBreakpoints()
nnoremap <silent> <Plug>VimspectorJumpToProgramCounter :call vimspector#JumpToProgramCounter()
nnoremap <silent> <Plug>VimspectorPause :call vimspector#Pause()
nnoremap <silent> <Plug>VimspectorRestart :call vimspector#Restart()
nnoremap <silent> <Plug>VimspectorStop :call vimspector#Stop()
nnoremap <silent> <Plug>VimspectorLaunch :call vimspector#Launch( v:true )
map <PageUp> <Plug>(SmoothieBackwards)
map <S-Up> <Plug>(SmoothieBackwards)
omap <C-B> <Plug>(SmoothieBackwards)
map <PageDown> <Plug>(SmoothieForwards)
map <S-Down> <Plug>(SmoothieForwards)
omap <C-F> <Plug>(SmoothieForwards)
map <C-U> <Plug>(SmoothieUpwards)
map <C-D> <Plug>(SmoothieDownwards)
noremap <silent> <Plug>(Smoothie_G) <Cmd>call smoothie#cursor_movement('G')  
noremap <silent> <Plug>(Smoothie_gg) <Cmd>call smoothie#cursor_movement('gg') 
noremap <silent> <Plug>(SmoothieBackwards) <Cmd>call smoothie#backwards()           
noremap <silent> <Plug>(SmoothieForwards) <Cmd>call smoothie#forwards()            
noremap <silent> <Plug>(SmoothieUpwards) <Cmd>call smoothie#upwards()             
noremap <silent> <Plug>(SmoothieDownwards) <Cmd>call smoothie#downwards()           
nnoremap <silent> <Plug>(grammarous-move-to-previous-error) :call grammarous#move_to_previous_error(getpos('.')[1 : 2], b:grammarous_result)
nnoremap <silent> <Plug>(grammarous-move-to-next-error) :call grammarous#move_to_next_error(getpos('.')[1 : 2], b:grammarous_result)
nnoremap <silent> <Plug>(grammarous-disable-category) :call grammarous#disable_category_at(getpos('.')[1 : 2], b:grammarous_result)
nnoremap <silent> <Plug>(grammarous-disable-rule) :call grammarous#disable_rule_at(getpos('.')[1 : 2], b:grammarous_result)
nnoremap <silent> <Plug>(grammarous-remove-error) :call grammarous#remove_error_at(getpos('.')[1 : 2], b:grammarous_result)
nnoremap <silent> <Plug>(grammarous-close-info-window) :call grammarous#info_win#close()
nnoremap <silent> <Plug>(grammarous-fixall) :call grammarous#fixall(b:grammarous_result)
nnoremap <silent> <Plug>(grammarous-fixit) :call grammarous#fixit(grammarous#get_error_at(getpos('.')[1 : 2], b:grammarous_result))
nnoremap <silent> <Plug>(grammarous-reset) :call grammarous#reset()
nnoremap <silent> <Plug>(grammarous-open-info-window) :call grammarous#create_update_info_window_of(b:grammarous_result)
nnoremap <silent> <Plug>(grammarous-move-to-info-window) :call grammarous#create_and_jump_to_info_window_of(b:grammarous_result)
nnoremap <silent> <C-K> :TmuxNavigateUp
nnoremap <silent> <C-J> :TmuxNavigateDown
nnoremap <silent> <C-H> :TmuxNavigateLeft
nnoremap <silent> <C-P> :Unite file_rec/neovim
tnoremap <silent> <C-`> :q
tnoremap <silent> <M-Right> :bnext
tnoremap <silent> <M-Left> :bprev
tnoremap <silent> <C-Down> :wincmd j
tnoremap <silent> <C-Up> :wincmd k
tnoremap <silent> <C-Left> :wincmd h
tnoremap <silent> <C-Right> :wincmd l
nnoremap <silent> <Plug>(nohlsearch) :nohlsearch
vnoremap <silent> <Plug>YankGitRemoteURL :call SpaceVim#util#CopyToClipboard(3)
nnoremap <silent> <Plug>YankGitRemoteURL :call SpaceVim#util#CopyToClipboard(2)
nnoremap <silent> <S-Tab> :wincmd p
nnoremap <silent> <F3> :Defx
noremap <silent> <F2> :TagbarToggle
vnoremap <silent> <Plug>ReverseLines :ReverseLines
nnoremap <silent> <Plug>ReverseLines :ReverseLines
nnoremap <silent> <F7> :MundoToggle
vnoremap <C-Space> <Plug>(wildfire-water)
xnoremap <silent> <Plug>(jplus :call dein#autoload#_on_map('<Plug>(jplus', 'vim-jplus','x')
nnoremap <silent> <Plug>(jplus :call dein#autoload#_on_map('<Plug>(jplus', 'vim-jplus','n')
xnoremap <silent> <Plug>(wildfire- :call dein#autoload#_on_map('<Plug>(wildfire-', 'wildfire.vim','x')
nnoremap <silent> <Plug>(wildfire- :call dein#autoload#_on_map('<Plug>(wildfire-', 'wildfire.vim','n')
xnoremap <silent> <Plug>SpaceVim-plugin-iedit :lua require('spacevim.plugin.iedit').start(1)
nnoremap <silent> <Plug>SpaceVim-plugin-iedit :lua require('spacevim.plugin.iedit').start()
vnoremap <C-S> :w
nnoremap <C-S> :w
nnoremap <silent> <Up> gk
nnoremap <silent> <Down> gj
xnoremap <S-Tab> <gv
nnoremap <silent> <C-`> :call SpaceVim#plugins#runner#close()
nnoremap <silent> <C-G> :call SpaceVim#plugins#ctrlg#display()
vnoremap <silent> <C-S-Up> :m '<-2gv=gv
vnoremap <silent> <C-S-Down> :m '>+1gv=gv
nnoremap <silent> <C-S-Up> :m .-2==
nnoremap <silent> <C-S-Down> :m .+1==
nnoremap <silent> <C-Down> :wincmd j
nnoremap <silent> <C-Up> :wincmd k
nnoremap <silent> <C-Left> :wincmd h
nnoremap <silent> <C-Right> :wincmd l
nnoremap <silent> <C-L> :TmuxNavigateRight
cnoremap  <Home>
inoremap <nowait> <silent> <expr>  coc#float#has_scroll() ? "\=coc#float#scroll(0)\" : "\<Left>"
cnoremap  <Left>
inoremap <silent> <expr>  coc#pum#visible() ? coc#pum#cancel() : "\"
inoremap <nowait> <silent> <expr>  coc#float#has_scroll() ? "\=coc#float#scroll(1)\" : "\<Right>"
cnoremap  <Right>
imap S <Plug>ISurround
imap s <Plug>Isurround
imap <silent> % <Plug>(matchup-c_g%)
inoremap <silent> <expr> 	 coc#pum#visible() ? coc#pum#next(1) : CheckBackspace() ? "\	" : coc#refresh()
cnoremap <expr>  repeat('<Del>', strchars(getcmdline()) - getcmdpos() + 1)
inoremap <silent> <expr>  coc#pum#visible() ? coc#pum#confirm(): "\u\\=coc#on_enter()\"
inoremap <silent> <expr>  coc#pum#visible() ? coc#pum#next(1) : "\"
inoremap <silent> <expr>  coc#pum#visible() ? coc#pum#prev(1) : "\"
imap  <Plug>Isurround
cnoremap  w
inoremap  u
imap  <Plug>(coc-snippets-expand-jump)
inoremap <silent> <expr>  coc#pum#visible() ? coc#pum#confirm() : "\"
imap <expr> ( pumvisible() ? has('patch-7.4.744') ? complete_parameter#pre_complete("()") : '(' : (len(maparg('<Plug>delimitMate(', 'i')) == 0) ? "\<Plug>delimitMate(" : '('
inoremap jk 
let &cpo=s:cpo_save
unlet s:cpo_save
set backupdir=~/.cache/SpaceVim/backup//
set completeopt=menu,menuone,longest
set cpoptions=aABceFs_d
set directory=~/.cache/SpaceVim/swap//
set expandtab
set fileencodings=utf-8,ucs-bom,gb18030,gbk,gb2312,cp936
set fillchars=vert:│,fold:·
set guicursor=n-v-c:block-blinkon10,i-ci-ve:ver25-blinkon10,r-cr:hor20,o:hor50,a:Cursor/lCursor
set guifont=SauceCodePro\ Nerd\ Font\ Mono:h11
set helplang=en
set listchars=tab:→\ ,eol:↵,trail:·,extends:↷,precedes:↶
set matchtime=0
set mouse=nv
set pumheight=15
set noruler
set runtimepath=~/.SpaceVim.d/,~/.SpaceVim/,~/.SpaceVim/bundle/tagbar,~/.cache/vimfiles/repos/github.com/Shougo/unite-outline,~/.SpaceVim/bundle/vim-grammarous,~/.SpaceVim/bundle/neomru.vim,~/.SpaceVim/bundle/nvim-if-lua-compat,~/.SpaceVim/bundle/vim-emoji,~/.SpaceVim/bundle/vim-cursorword,~/.SpaceVim/bundle/vim-clipboard,~/.SpaceVim/bundle/vim-smoothie,~/.SpaceVim/bundle/defx-icons,~/.SpaceVim/bundle/neosnippet-snippets,~/.cache/vimfiles/repos/github.com/osyo-manga/unite-quickfix,~/.SpaceVim/bundle/vim-textobj-line,~/.SpaceVim/bundle/hop.nvim,~/.cache/vimfiles/repos/github.com/puremourning/vimspector,~/.SpaceVim/bundle/vim-textobj-indent,~/.SpaceVim/bundle/nvim-treesitter,~/.cache/vimfiles/repos/github.com/neoclide/coc.nvim,~/.SpaceVim/bundle/vim-repeat,~/.SpaceVim/bundle/vim-matchup,~/.SpaceVim/bundle/gruvbox,~/.SpaceVim/bundle/CompleteParameter.vim,~/.SpaceVim/bundle/unite.vim,~/.cache/vimfiles/repos/github.com/edkolev/tmuxline.vim,~/.SpaceVim/bundle/editorconfig-vim,~/.SpaceVim/bundle/delimitMate,~/.SpaceVim/bundle/vim-expand-region,~/.cache/vimfiles/repos/github.com/Shougo/vimproc.vim,~/.SpaceVim/bundle/vim-textobj-user,~/.SpaceVim/bundle/indent-blankline.nvim,~/.SpaceVim/bundle/open-browser.vim,~/.SpaceVim/bundle/tagbar-proto.vim,~/.SpaceVim/bundle/unite-sources,~/.SpaceVim/bundle/defx.nvim,~/.cache/vimfiles/repos/github.com/tmux-plugins/vim-tmux-focus-events,~/.SpaceVim/bundle/tabular,~/.SpaceVim/bundle/defx-sftp,~/.SpaceVim/bundle/vim-startify,~/.SpaceVim/bundle/vim-van,~/.SpaceVim/bundle/tagbar-makefile.vim,~/.SpaceVim/bundle/neoyank.vim,~/.cache/vimfiles/repos/github.com/roxma/vim-tmux-clipboard,~/.SpaceVim/bundle/vim-markdown-toc,~/.SpaceVim/bundle/dein.vim,~/.SpaceVim/bundle/vim-table-mode,~/.cache/vimfiles/repos/github.com/lvht/tagbar-markdown,~/.SpaceVim/bundle/neoformat,~/.SpaceVim/bundle/vim-textobj-entire,~/.SpaceVim/bundle/vim-surround,~/.SpaceVim/bundle/deoplete-dictionary,~/.SpaceVim/bundle/clever-f.vim,~/.SpaceVim/bundle/nerdcommenter,~/.SpaceVim/bundle/defx-git,~/.SpaceVim/bundle/fcitx.vim,~/.SpaceVim/bundle/neosnippet.vim,~/.SpaceVim/bundle/neopairs.vim,~/.SpaceVim/bundle/neoinclude.vim,~/.SpaceVim/bundle/context_filetype.vim,~/.SpaceVim/bundle/neco-syntax,~/.SpaceVim/bundle/vim-snippets,~/.cache/vimfiles/repos/github.com/Shougo/echodoc.vim,~/.SpaceVim/bundle/vim-choosewin,~/.cache/vimfiles/.cache/init.vim/.dein,/usr/share/nvim/runtime,~/.SpaceVim/bundle/vim-matchup/after,~/.SpaceVim/bundle/CompleteParameter.vim/after,~/.SpaceVim/bundle/tabular/after,~/.cache/vimfiles/.cache/init.vim/.dein/after,~/.SpaceVim.d/after,~/.SpaceVim/bundle/dein.vim/,~/.SpaceVim/after,~/.config/coc/extensions/node_modules/coc-snippets
set scrolloff=1
set noshelltemp
set shiftwidth=2
set shortmess=ltnfsTcixoOF
set noshowcmd
set showmatch
set noshowmode
set showtabline=2
set sidescrolloff=5
set softtabstop=2
set tabline=%!SpaceVim#layers#core#tabline#get()
set tabstop=2
set termguicolors
set undodir=~/.cache/SpaceVim/undofile//
set undofile
set wildignore=*/tmp/*,*.so,*.swp,*.zip,*.class,tags,*.jpg,*.ttf,*.TTF,*.png,*/target/*,.git,.svn,.hg,.DS_Store,*.svg
set wildignorecase
set window=52
set nowritebackup
" vim: set ft=vim :
