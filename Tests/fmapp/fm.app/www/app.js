let seq = 0;
const pending = new Map();
window.__fmCb = (r) => {
  const p = pending.get(r.id);
  if (!p) return;
  pending.delete(r.id);
  r.ok ? p.resolve(r.data) : p.reject(new Error(r.err || "error"));
};
function fm(op, args = {}) {
  return new Promise((resolve, reject) => {
    const id = ++seq;
    pending.set(id, {resolve, reject});
    window.webkit.messageHandlers.fm.postMessage({ id, op, ...args });
  });
}

let cur = "/";
let editing = null;
const $ = s => document.querySelector(s);
const join = (a, b) => a.endsWith("/") ? a + b : a + "/" + b;

async function list(path = cur) {
  const data = await fm("list", { path });
  cur = path;
  $("#path").value = data.abs;
  const box = $("#list");
  box.innerHTML = "";
  for (const e of data.entries) {
    const d = document.createElement("div");
    d.className = "entry";
    const n = document.createElement("span");
    n.className = "name " + (e.dir ? "dir" : "file");
    n.textContent = e.name + (e.dir ? "/" : "") + (e.link ? " →" : "");
    n.onclick = () => e.dir ? list(join(data.abs, e.name))
                            : openFile(join(data.abs, e.name));
    d.appendChild(n);
    if (!e.dir) {
      const s = document.createElement("span");
      s.className = "size";
      s.textContent = fmtSize(e.size);
      d.appendChild(s);
    }
    const del = document.createElement("button");
    del.textContent = "Delete";
    del.onclick = async () => {
      if (!confirm("Delete " + e.name + " ?")) return;
      try { await fm("rm", { path: join(data.abs, e.name) }); refresh(); }
      catch (err) { alert(err.message); }
    };
    d.appendChild(del);
    box.appendChild(d);
  }
}

function fmtSize(n) {
  if (n < 1024) return n + " B";
  if (n < 1024*1024) return (n/1024).toFixed(1) + " KB";
  return (n/1024/1024).toFixed(1) + " MB";
}
function up() {
  if (cur === "/") return;
  list(cur.split("/").slice(0, -1).join("/") || "/");
}
function refresh() { list(cur).catch(e => alert(e.message)); }

async function newDir() {
  const n = prompt("Directory name");
  if (!n) return;
  try { await fm("mkdir", { path: join(cur, n) }); refresh(); }
  catch (e) { alert(e.message); }
}

async function openFile(path) {
  try {
    const text = await fm("read", { path });
    editing = path;
    $("#editor").value = text;
    $("#editor").style.display = "block";
    $("#saveBtn").style.display = "inline";
    $("#closeBtn").style.display = "inline";
  } catch (e) { alert("Failed to read: " + e.message); }
}
async function save() {
  if (!editing) return;
  try { await fm("write", { path: editing, data: $("#editor").value }); alert("Saved"); }
  catch (e) { alert("Failed to save: " + e.message); }
}
function closeEditor() {
  editing = null;
  $("#editor").style.display = "none";
  $("#saveBtn").style.display = "none";
  $("#closeBtn").style.display = "none";
}

function go() { list($("#path").value || "/").catch(e => alert(e.message)); }

list("/").catch(e => alert("init failed: " + e.message));

