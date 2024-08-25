class SynoptiqueComponent extends HTMLElement {
    constructor() {
        super();
        this.attachShadow({ mode: 'open' });
        this.shadowRoot.innerHTML = `
            <object id="syno" type="image/svg+xml" data="cuves.svg"></object>
        `;
        
        this.svgObject = this.shadowRoot.querySelector('#syno');
        this.svgDocument = null;

        // Add an event listener to wait for the SVG to load
        this.svgObject.addEventListener('load', async () => {
            this.svgDocument = this.svgObject.contentDocument;
        });
    }

    async waitForSVGLoad() {
        return new Promise((resolve) => {
            if (this.svgDocument) {
                resolve(this.svgDocument);
            } else {
                this.svgObject.addEventListener('load', () => {
                    resolve(this.svgObject.contentDocument);
                });
            }
        });
    }

    async update_cuves(synoptique_datas) {
        const svg = await this.waitForSVGLoad();
        if (!svg) return;

        console.log('done');
        console.log(synoptique_datas);

        this.update_cuve(
            svg,
            synoptique_datas['main'],
            'niveau_main_1',
            'niveau_main_2',
            'niveau_main_3',
            'volt_main',
            'db_main',
            'module_main',
            'date_main',
            'heure_main'
        );

        this.update_cuve(
            svg,
            synoptique_datas['paul'],
            'niveau_paul_1',
            'niveau_paul_2',
            'niveau_paul_3',
            'volt_paul',
            'db_paul',
            'module_paul',
            'date_paul',
            'heure_paul'
        );

        this.update_cuve(
            svg,
            synoptique_datas['reduit'],
            'niveau_reduit_1',
            'niveau_reduit_2',
            'niveau_reduit_3',
            'volt_reduit',
            'db_reduit',
            'module_reduit',
            'date_reduit',
            'heure_reduit'
        );

        this.update_cuve(
            svg,
            synoptique_datas['barbec'],
            'niveau_barbec_1',
            'niveau_barbec_2',
            'niveau_barbec_3',
            'volt_barbec',
            'db_barbec',
            'module_barbec',
            'date_barbec',
            'heure_barbec'
        );

        this.update_pompe(svg, synoptique_datas['main'], 'pompe_main');
        this.update_pompe(svg, synoptique_datas['paul'], 'pompe_paul');
        this.update_pompe(svg, synoptique_datas['reduit'], 'pompe_reduit');
    }

    update_cuve_level(svg, datas, id_l1, id_l2, id_l3) {
		console.log(datas)
		
        const level = datas['level'] || 0;
        const l1 = this.get_id(svg, id_l1);
        const l2 = this.get_id(svg, id_l2);
        const l3 = this.get_id(svg, id_l3);

        if (!l1 || !l2 || !l3) return;

        switch (level) {
            case 1:
                l1.style.display = "block";
                l2.style.display = "none";
                l3.style.display = "none";
                break;
            case 2:
                l1.style.display = "block";
                l2.style.display = "block";
                l3.style.display = "none";
                break;
            case 3:
                l1.style.display = "block";
                l2.style.display = "block";
                l3.style.display = "block";
                break;
            case 0:
            default:
                l1.style.display = "none";
                l2.style.display = "none";
                l3.style.display = "none";
        }
    }

    formatDate(dtme) {
        if (!dtme) return null;
        const date = new Date(dtme);
        const day = String(date.getUTCDate()).padStart(2, '0');
        const month = String(date.getUTCMonth() + 1).padStart(2, '0');
        const year = date.getUTCFullYear();
        return `${day}/${month}/${year}`;
    }

    formatTime(dtme) {
        if (!dtme) return null;
        const date = new Date(dtme);
        const hours = String(date.getUTCHours()).padStart(2, '0');
        const minutes = String(date.getUTCMinutes()).padStart(2, '0');
        const seconds = String(date.getUTCSeconds()).padStart(2, '0');
        return `${hours}:${minutes}:${seconds}`;
    }

    update_wiio_module(svg, datas, id_volt, id_db, id_module, id_date, id_time) {
        const volt = datas['volt'] || 0;
        let db = datas['db'] || 0;
        const state = datas['state'] || 'unknown';

        const txtVolt = this.get_id(svg, id_volt);
        if (txtVolt) txtVolt.textContent = `${(volt / 10).toString()} V`;

        if (db & (1 << 15)) db -= 65536;
        const txtDb = this.get_id(svg, id_db);
        if (txtDb) txtDb.textContent = `${db} dB`;

        const rect = this.get_id(svg, id_module);
        if (rect) {
            rect.style.fill = state === 'sleep' ? '#000000' : state === 'on' ? '#018004' : 'gray';
        }

        const txtDate = this.get_id(svg, id_date);
        if (txtDate) {
            const dte = this.formatDate(datas['time']);
            txtDate.textContent = dte || '__/__/____';
        }

        const txtTime = this.get_id(svg, id_time);
        if (txtTime) {
            const tme = this.formatTime(datas['time']);
            txtTime.textContent = tme || '__:__:__';
        }

        if (rect) {
            const tooltip = document.createElementNS("http://www.w3.org/2000/svg", "title");
            tooltip.textContent = datas['time'];
            rect.appendChild(tooltip);
        }
    }

    update_cuve(svg, datas, id_l1, id_l2, id_l3, id_volt, id_db, id_module, id_date, id_time) {
		console.log(datas)
        this.update_cuve_level(svg, datas, id_l1, id_l2, id_l3);
        this.update_wiio_module(svg, datas, id_volt, id_db, id_module, id_date, id_time);
    }

    update_pompe(svg, datas, id_pompe) {
        const on = datas['pompe'];
        const pompe = this.get_id(svg, id_pompe);
        const left = this.get_id(svg, `${id_pompe}_left`);
        const right = this.get_id(svg, `${id_pompe}_right`);

        if (pompe && left && right) {
            pompe.style.fill = on ? '#0000FF' : '#000000';
            left.style.stroke = on ? '#0000FF' : '#000000';
            right.style.stroke = on ? '#0000FF' : '#000000';
        }
    }

    get_id(svg, id_name) {
        const element = svg.getElementById(id_name);
        if (!element) {
            console.error(`${id_name} Non trouvé sur le SVG`);
        }
        return element;
    }
}

customElements.define('synoptique-component', SynoptiqueComponent);

// Exposer la fonction update_cuves
window.update_cuves = function(data) {
    const component = document.querySelector('synoptique-component');
    if (component) {
        component.update_cuves(data);
    }
};
